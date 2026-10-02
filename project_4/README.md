# Invoke For All

Implement `invoke_forall`, a `constexpr` function template that extends
`std::invoke` to perform multiple calls using corresponding elements of
tuple-like arguments.

An argument is **gettable** if its underlying type has a `std::tuple_size`
specialization derived from `std::integral_constant` and every element can be
accessed with `std::get<i>`.

## Invocation rules

- With no gettable arguments, behave like a single call to `std::invoke`.
- Otherwise, all gettable arguments must have the same number of elements,
  `m`. Perform `m` calls, using element `i` of each gettable argument for call
  `i`, and reusing ordinary arguments across calls. This also allows a tuple
  of callables as the first argument.
- Provide `protect_arg(arg)` to pass a gettable object as a whole argument
  instead of expanding its elements.

## Results and forwarding

Return the collected results in an object accessible with `std::get<i>`. When
all calls have the same result type, this object must also satisfy
`std::ranges::random_access_range`. Results returned by lvalue reference must
remain accessible for modifying the referenced objects. Accessing the result
of a `void` call is undefined.

Preserve perfect forwarding, including for protected arguments. When an
ordinary rvalue argument is reused across calls, forward it only once and
make copies for the other calls.

Require at least one argument using a concept. The entire operation must be
usable in constant expressions whenever all underlying calls can be evaluated
at compile time.

*Condensed English summary of the original assignment.*
