# Text Moths

Implement a simulation of moths feeding on text. Each text is a circular sequence
of printable ASCII characters, and each moth has a position, vitality, and a
movement parameter `P`.

## Feeding rules

During each feeding cycle, active moths act in the order they were added. A moth
first moves, wrapping around the text, then tries to eat the character at its
new position.

- Movement costs 10 vitality points per position. If the moth cannot afford the
  full move, it stays in place, loses its remaining vitality, and becomes inactive.
- Eating a character adds its ASCII value to the moth's vitality and replaces
  the character with a space.
- Landing on a space costs 32 vitality points, without reducing vitality below
  zero.

| Type | Eats | Movement per cycle |
| --- | --- | --- |
| Common (`*`) | Any character with ASCII code 33–126 | `P` positions |
| Letter (`A`) | Uppercase and lowercase letters | `P` positions |
| Digit (`1`) | Digits | `P` positions |
| Picky (`!`) | Characters other than letters and digits, excluding spaces | Repeated sequence of 1, 2, …, `P` positions |

## Commands

Read one command per line from standard input:

| Command | Action |
| --- | --- |
| `TEXT T text` | Create a text with ID `T`. |
| `MOTH T N R V P` | Add a moth of type `R` at position `N`, with vitality `V` and parameter `P`. |
| `FEED T C` | Run `C` feeding cycles on text `T`. |
| `PRINTM T` | Print each moth's type, parameter, position, and vitality in insertion order. |
| `PRINTT T` | Print the current text. |
| `DELETE T` | Delete the text and its moths. |

Validate command syntax and parameters, including duplicate text IDs, missing
texts, and out-of-range positions. Ignore invalid lines and print `ERROR L` to
standard error, where `L` is the line number starting at 1.

Use object-oriented design and split the implementation into C++ modules,
without preprocessor directives.

*Condensed English summary of the original assignment.*
