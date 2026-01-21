#ifndef INVOKE_FORALL_H
#define INVOKE_FORALL_H
#include <algorithm>
#include <array>
#include <concepts>
#include <functional>
#include <iostream>
#include <limits>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <utility>

namespace detail {

template<typename T>
concept TupleLike = requires {
    typename std::tuple_size<std::remove_cvref_t<T>>::type;
    requires std::derived_from<
        std::tuple_size<std::remove_cvref_t<T>>,
        std::integral_constant<size_t, std::tuple_size_v<std::remove_cvref_t<T>>>
    >;
};

template<typename T>
concept Gettable = TupleLike<T> && 
    []<size_t... Ids> (std::index_sequence<Ids...>) {
        return requires (T t){ (std::get<Ids>(t), ...); };
    } (std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<T>>>{});

// ProtectedGettable and utils
template<typename T>
class ProtectedGettable {
public:
    std::tuple<T> value;

    constexpr ProtectedGettable(T& _value)
        : value(std::forward_as_tuple(std::forward<T>(_value))) {}

    constexpr ProtectedGettable(T&& _value)
        requires (!std::is_lvalue_reference_v<T>)
        : value(std::make_tuple(std::forward<T>(_value))) {}

    constexpr decltype(auto) get_value() { return std::get<0>(value); }
};

template<typename T>
struct is_protected_gettable : std::false_type{};

template<typename T>
struct is_protected_gettable<ProtectedGettable<T>> : std::true_type{};

/*
Helper function to retrieve the arity of a given object. If the object is
Gettable it returns its arity, otherwise it returns fallback argument.
*/
template<typename T>
constexpr size_t helper_get_arity(size_t fallback) {
    using RawT = std::remove_cvref_t<T>;
    if constexpr (Gettable<T> 
        && !is_protected_gettable<RawT>::value)
            return std::tuple_size_v<RawT>;
    else
        return fallback;
}

/*
Checks and asserts whether all of the Gettable objects have a common arity.
If there are no Gettable objects returns 0, otherwise returns the common arity.
*/
template<typename... Args>
constexpr size_t calculate_arity() {
    constexpr bool no_gettable = !(Gettable<Args> || ...);
    if constexpr (no_gettable) return 0;
    else {
        constexpr size_t max_size = std::max({
            helper_get_arity<Args>(0)...
        });
        constexpr size_t min_size = std::min({
            helper_get_arity<Args>(std::numeric_limits<size_t>::max())...
        });
        static_assert(max_size == min_size);
        return max_size;
    }
}

/*
Helper to extract the raw component preserving value category.
Responsible for the logic deciding whether to return a whole argument or 
access a component with std::get.
*/
template<size_t Idx, typename T>
constexpr decltype(auto) extract_component(T&& arg) {
    using RawT = std::remove_cvref_t<T>;
    if constexpr (Gettable<T> && !is_protected_gettable<RawT>::value)
        return std::get<Idx>(std::forward<T>(arg));
    else if constexpr (is_protected_gettable<RawT>::value)
        return std::get<0>(std::forward<T>(arg).value);
    else
        return std::forward<T>(arg);
}

/*
Helper responsible for judging whether we should pass the argument by copy
or to std::forward it.
*/
template<size_t Idx, size_t M, bool ForceCopy, typename T>
constexpr decltype(auto) get_arg(T&& arg) {
    if constexpr (Gettable<T> || Idx == M-1) {
        return extract_component<Idx>(std::forward<T>(arg));
    }
    // We forcibly copy to create a temporary r-value
    else if constexpr (ForceCopy) { 
        decltype(auto) val = extract_component<Idx>(std::forward<T>(arg));
        return std::remove_cvref_t<decltype(val)>(val);
    }
    else {
        return extract_component<Idx>(arg);
    }
}

/*
Performs std::invoke on provided arguments, calling different versions depending
on a provided function.
*/
template<size_t Idx, size_t M, typename... Args>
constexpr decltype(auto) invoke_idx_value(Args&&... args) {
    // If the function does not require an explicit r-value in argument, we dont need to force it
    if constexpr (requires { std::invoke(get_arg<Idx, M, false>(std::forward<Args>(args))...); })
        return std::invoke(get_arg<Idx, M, false>(std::forward<Args>(args))...);
    else 
        return std::invoke(get_arg<Idx, M, true>(std::forward<Args>(args))...);
}

/*
Returns either std::monostate or a result std::invoke on provided arguments, with respect
to the function's return value
*/
template<size_t Idx, size_t M, typename... Args>
constexpr decltype(auto) invoke_idx(Args&&... args) {
    using ReturnType = decltype(invoke_idx_value<Idx, M>(std::forward<Args>(args)...));
    if constexpr (std::is_same_v<ReturnType, void>) {
        invoke_idx_value<Idx, M>(std::forward<Args>(args)...);
        return std::monostate();
    }
    else
        return invoke_idx_value<Idx, M>(std::forward<Args>(args)...);
}

} // namespace

template<typename T>
constexpr decltype(auto) protect_arg(T&& arg) {
    return detail::ProtectedGettable<T>(std::forward<T>(arg));
}

template<typename... Args>
requires (sizeof... (Args) > 0)
constexpr decltype(auto) invoke_forall(Args&&... args) {
    constexpr size_t arity = detail::calculate_arity<Args...>();
    if constexpr (!arity) {
        return detail::invoke_idx<0, 1>(std::forward<Args>(args)...);
    }
    else {
        using TargetReturnType = decltype(detail::invoke_idx<0, arity>(std::forward<Args>(args)...));
        constexpr bool all_types_equal = [&]<size_t... Ids> (std::index_sequence<Ids...>) {
            return (std::is_same_v<
                decltype(detail::invoke_idx<Ids, arity>(std::forward<Args>(args)...)),
                TargetReturnType
            > && ...);
        } (std::make_index_sequence<arity>{});

        if constexpr (!all_types_equal) {
            return [&]<size_t... Ids> (std::index_sequence<Ids...>) {
                return std::tuple<decltype(detail::invoke_idx<Ids, arity>(std::forward<Args>(args)...))...>(
                    detail::invoke_idx<Ids, arity>(std::forward<Args>(args)...)...
                );
            } (std::make_index_sequence<arity>{});
        }
        else if constexpr (std::is_lvalue_reference_v<TargetReturnType>) {
            return [&]<size_t... Ids> (std::index_sequence<Ids...>) {
                return std::array<std::reference_wrapper<std::remove_reference_t<TargetReturnType>>, arity>{
                    detail::invoke_idx<Ids, arity>(std::forward<Args>(args)...)...
                };
            } (std::make_index_sequence<arity>{});
        }
        else {
            return [&]<size_t... Ids> (std::index_sequence<Ids...>) {
                return std::array{
                    detail::invoke_idx<Ids, arity>(std::forward<Args>(args)...)...
                };
            } (std::make_index_sequence<arity>{});
        }
    }
}

#endif