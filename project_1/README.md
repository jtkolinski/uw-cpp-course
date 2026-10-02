# Mastermind

Implement an interactive Mastermind game in which the program can play either
the codemaker or the codebreaker. The codemaker chooses a secret sequence of
colored pegs, and the codebreaker tries to discover it through successive guesses.

Each guess receives two counts: pegs with the correct color and position, and
pegs with the correct color in the wrong position. The game ends when every
position matches the secret. Colors are represented by integers starting at 0.

## Game modes

- **Codebreaker:** with two command-line arguments (the number of colors and
  the sequence length), the program generates guesses and reads the user's
  feedback. It must use a reasonable guessing strategy.
- **Codemaker:** with more than two arguments, the first specifies the number
  of colors and the rest define the secret sequence. The program reads the
  user's guesses and returns feedback.

Exchange guesses and feedback through standard input and output, using one line
per message and single spaces between numbers, with no additional whitespace.

## Error handling

Detect invalid arguments, malformed input, and feedback inconsistent with every
possible secret. On error, print `ERROR` to standard error and exit with status
1. A completed game or closed input stream ends with status 0.

Use standard library containers and algorithms for data handling, and C++
streams for input and output.

*Condensed English summary of the original assignment.*
