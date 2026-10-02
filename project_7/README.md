# Event Streams

Implement a functional C++ library for processing event streams while carrying
state between events.

An observer receives an event and the current state, then returns an updated
state and either `control::continue_` or `control::stop`. Calling a stream with
an observer and an initial state processes events sequentially and returns the
final state. Processing stops immediately when requested by the observer or
when the stream runs out of events.

## Stream operations

| Operation | Behavior |
| --- | --- |
| `emit(x)` | Emit a single event. |
| `generate(init, step)` | Emit `init`, then repeatedly apply `step` until it returns `std::nullopt`. |
| `counter()` | Emit integers starting at 1, wrapping from `INT_MAX` to `INT_MIN`. |
| `map(f, s)` | Transform each event using `f`. |
| `filter(pred, s)` | Forward only events satisfying the predicate. |
| `take(n, s)` | Forward at most the first `n` events, then stop consuming the source. |
| `flatten(ss)` | Process a stream of streams in sequence. |
| `tap(side_effect)` | Run a side effect for each event without changing the event or stop signal. |

Support composition with `|`, such as `s | map(f) | filter(pred) | take(n)`.

Also implement `memoize(f)`: cache results by argument values and reuse them on
repeated calls. Preserve implicit argument conversions supported by `f`.

Copies of counters and memoized functions must have independent state and
caches. Events, state, and supplied callables may be assumed copyable. Use
lambdas for stream operations and expose the library in namespace `eventstream`.

*Condensed English summary of the original assignment.*
