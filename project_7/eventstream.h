#ifndef EVENTSTREAM_H
#define EVENTSTREAM_H

#include <climits>
#include <map>
#include <tuple>
#include <type_traits>

namespace eventstream {

enum class control { continue_, stop };

namespace detail {

// Traits for extracting function signature from callable types.
template<typename T>
struct callable_traits : callable_traits<decltype(&T::operator())> {}; 

#define QUAL_VARIANT(F, CV) \
    F(CV) F(CV noexcept) \
    F(CV &) F(CV & noexcept) \
    F(CV &&) F(CV && noexcept)

#define FOR_ALL_QUALS(F) \
    QUAL_VARIANT(F, ) QUAL_VARIANT(F, const) QUAL_VARIANT(F, volatile) QUAL_VARIANT(F, const volatile)

#define SPECIALIZE_MEMBERS(QUALS) \
    template<typename R, typename C, typename... Args> \
    struct callable_traits<R (C::*)(Args...) QUALS> { \
        using return_type = R; \
        using args_tuple = std::tuple<std::remove_cvref_t<Args>...>; \
    };

// All of functor/lambda combinations
FOR_ALL_QUALS(SPECIALIZE_MEMBERS)

// Plain Functions

template<typename R, typename... Args>
struct callable_traits<R(*)(Args...)> {
    using return_type = R;
    using args_tuple = std::tuple<std::remove_cvref_t<Args>...>;
};

template<typename R, typename... Args>
struct callable_traits<R(*)(Args...) noexcept> {
    using return_type = R;
    using args_tuple = std::tuple<std::remove_cvref_t<Args>...>;
};

#undef SPECIALIZE_MEMBERS
#undef FOR_ALL_QUALS
#undef QUAL_VARIANT

template<typename F>
struct memoize_helper {
    using return_type = typename callable_traits<F>::return_type;
    using args_tuple = typename callable_traits<F>::args_tuple;

    mutable std::map<args_tuple, return_type> cache;
    F callable;

    memoize_helper(const F& f) : callable(f) {}

    template<typename... Args>
    decltype(auto) operator()(Args&&... args) const {
        args_tuple key{ std::forward<Args>(args)... };

        if (auto it = cache.find(key); it != cache.end()) {
            return (it->second);
        }

        return_type result = callable(std::forward<Args>(args)...);
        auto [it, inserted] = cache.emplace(key, result); 
        return (it->second);
    }
};

template<typename Event, typename EventGenerator, typename Obs, typename State>
State run_stream(Event event, EventGenerator& event_generator, Obs& observer, State state) {
    auto [new_state, obs_instruction] = observer(event, state);
    if (obs_instruction == control::stop) return new_state;

    auto [new_event, gen_instruction] = event_generator(event);
    if (gen_instruction == control::stop) return new_state;

    return run_stream(new_event, event_generator, observer, new_state);
}

auto stream = [](auto init, auto event_generator) {
    return [init, event_generator]<typename Obs, typename State>(Obs observer, State state)
            mutable -> State {
        auto [event, init_instruction] = event_generator(init, true);
        return init_instruction == control::stop 
               ? state 
               : run_stream(event, event_generator, observer, state);
    };
};

} // namespace detail

// Streams

[[maybe_unused]] auto emit = [](auto x) {
    auto event_generator = [](auto event, bool first_event = false) {
        return first_event ? std::make_pair(event, control::continue_)
                           : std::make_pair(event, control::stop);
    };
    return detail::stream(x, event_generator);
};

[[maybe_unused]] auto generate = [](auto init, auto step) {
    auto event_generator = [step](auto event, bool first_event = false) {
        if (first_event)
             return std::make_pair(event, control::continue_); 

        auto next_opt = step(event);
        return !next_opt.has_value()
               ? std::make_pair(event, control::stop)
               : std::make_pair(*next_opt, control::continue_);
    };
    return detail::stream(init, event_generator);
};

[[maybe_unused]] auto counter = []() {
    int n = 0;
    auto event_generator = [n](
        [[maybe_unused]] auto event, [[maybe_unused]] bool first_event = false
    ) mutable {
        n = (n == INT_MAX) ? INT_MIN : n + 1;
        return std::make_pair(n, control::continue_);
    };
    return detail::stream(n, event_generator);
};

// Stream Transformers

auto map(auto f, auto s) {
    return [=]<typename Obs, typename State>(Obs observer, State state) mutable -> State {
        auto wrapped_observer = [f, observer](auto event, auto current_state) mutable {
            return observer(f(event), current_state);
        };
        return s(wrapped_observer, state);
    };
}

auto filter(auto pred, auto s) {
    return [=]<typename Obs, typename State>(Obs observer, State state) mutable -> State {
        auto wrapped_observer = [pred, observer](auto event, auto current_state) mutable {
            return pred(event)
                   ? observer(event, current_state)
                   : decltype(observer(event, current_state)){current_state, control::continue_};
        };
        return s(wrapped_observer, state);
    };
}

auto take(std::size_t n, auto s) {
    return [=]<typename Obs, typename State>(Obs observer, State state) mutable -> State {
        if (n == 0) return state;
        auto wrapped_observer = [n, observer](auto event, auto current_state) mutable {
            auto result = observer(event, current_state);
            if (--n == 0) result.second = control::stop;
            return result;
        };
        return s(wrapped_observer, state);
    };
}

auto flatten(auto ss) {
    return [=]<typename Obs, typename State>(Obs observer, State state) mutable -> State {
        auto wrapped_observer = [observer](auto inner_stream, State current_state) mutable
            -> std::pair<State, control> {
            control last_instruction = control::continue_;
            auto tracking_observer = [&](auto event, State inner_state)
                -> std::pair<State, control> { 
                auto result = observer(event, inner_state); 
                last_instruction = result.second; 
                return result;
            };
            State new_state = inner_stream(tracking_observer, current_state);
            return std::make_pair(new_state, last_instruction);
        };
        return ss(wrapped_observer, state);
    };
}

auto map(auto f) {
    return [=](auto s) {
        return map(f, s);
    };
}

auto filter(auto pred) {
    return [=](auto s) {
        return filter(pred, s);
    };
}

auto take(size_t n) {
    return [=](auto s) {
        return take(n, s);
    };
}

auto flatten() {
    return [=](auto ss) {
        return flatten(ss);
    };
}

// Helper tools

decltype(auto) operator | (auto s, auto transform) {
    return transform(s);
}

auto tap(auto side_effect) {
    return [=](auto s) { 
        return [=]<typename Obs, typename State>(Obs observer, State state) mutable -> State {
            auto wrapped_observer = [side_effect, observer](auto event, auto current_state) mutable {
                side_effect(event);
                return observer(event, current_state);
            };
            return s(wrapped_observer, state);
        };
    };
}

auto memoize(auto f) {
    return detail::memoize_helper<decltype(f)>(f);
}

} // namespace eventstream

#endif