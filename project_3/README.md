# Fruit Picking

Implement a library that models fruit pickers and ranks them by their harvests.
The interface consists of three classes: `Fruit`, `Picker`, and `Ranking`.

## Fruit and pickers

A `Fruit` has three strongly typed attributes:

- **Taste:** sweet or sour.
- **Size:** large, medium, or small.
- **Quality:** healthy, rotten, or worm-infested.

Support equality, explicit conversion to and from a tuple of attributes, and
`constexpr` construction and read-only operations.

A `Picker` stores a name and fruits in collection order. Pickers can collect
fruit, take another picker's earliest fruit, give away their own earliest fruit,
and count fruits by each attribute.

Adding a fruit can affect the harvest:

- A healthy fruit added after a rotten one becomes rotten.
- Adding a rotten fruit makes the previous fruit rotten if it was healthy.
- Adding a worm-infested fruit infects all previously collected fruits that are
  both healthy and sweet.

The same rules apply when transferring fruit between pickers.

## Ranking

Rank pickers by the number of healthy fruits, then sweet fruits, then large,
medium, small, and finally total fruits. At each step, more is better. Ties
preserve insertion order. Equality is separate: two pickers are equal only if
their names and ordered fruit collections match.

Support adding pickers, removing the highest-ranked matching picker, merging
rankings, and read-only access by position. Provide copy and move semantics and
stream output for all three classes.

*Condensed English summary of the original assignment.*
