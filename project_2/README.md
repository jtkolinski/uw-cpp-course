# Named Poset Collections

Implement a C++ library for managing collections of named partially ordered sets
(posets), with an interface usable from both C and C++.

Each poset contains the integers from `0` to `N - 1` and a relation that must
remain reflexive, antisymmetric, and transitive. A newly created poset contains
only the reflexive pairs `(x, x)`.

## Main requirements

- Create and delete collections, identifying each new collection with a unique
  numeric ID.
- Create, delete, and copy named posets within a collection. Names must be
  nonempty and contain only English letters, digits, or underscores.
- Traverse posets in ASCII lexicographic order of their names.
- Add and query relations. Adding a relation must preserve the partial order
  and include all pairs implied by transitivity.
- Remove a non-reflexive pair `(x, y)` only if no intermediate element `z`
  satisfies both `x <= z` and `z <= y`.
- Report the number of collections, the number of posets in a collection, and
  the number of elements in each poset.

Use standard library containers rather than custom data-holding classes or
structures. Collections must own copies of their names. Expose the C++ interface
in namespace `cxx` and keep internal state and helper functions private to the
implementation.

*Condensed English summary of the original assignment.*
