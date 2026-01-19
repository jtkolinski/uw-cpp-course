#include <charconv>
#include <exception>
#include <iostream>
#include <numeric>
#include <ranges>
#include <set>
#include <unordered_map>
#include <vector>

namespace
{

constexpr size_t MIN_NUM_COLORS = 2;
constexpr size_t MAX_NUM_COLORS = 256;
constexpr size_t MIN_SEQUENCE_LEN = 2;
constexpr size_t MAX_SEQUENCE_LEN = 10;
constexpr uint64_t MAX_COMBINATIONS = 1ll << 24;

void exit_with_error()
{
    std::cerr << "ERROR\n";
    std::exit(1);
}

class NormalExitException : public std::exception {};

// The function converts a string to size_t, checking if the value
// is within the range [min_val, max_val].
// If the conversion fails or the value is out of range,
// it causes the program to terminate with an error.
size_t convert_input(std::string_view input, size_t min_val, size_t max_val)
{
    size_t value = 0;
    auto result =
        std::from_chars(input.data(), input.data() + input.size(), value);

    if (result.ec != std::errc{}) {
        exit_with_error();
    }
    else if (result.ptr != input.data() + input.size()) {
        exit_with_error();
    }
    else if (value < min_val or value > max_val) {
        exit_with_error();
    }

    return value;
}

// The function reads a line from standard input and parses it as
// a vector of integers separated by spaces.
// Each value must be in the range [0, max_val].
// The vector must have exactly 'len' elements.
std::vector<uint8_t> input_vector(size_t len, uint8_t max_val)
{
    std::string input;
    std::getline(std::cin, input);

    if (std::cin.eof() and input.empty()) {
        throw NormalExitException();
    }
    if (std::cin.fail()) {
        exit_with_error();
    }
    if (input.ends_with('\r')) {
        input.pop_back();
    }

    std::vector<uint8_t> converted_input =
        input | std::views::split(' ') |
        std::views::transform([&](auto &&word_obj) {
            std::string word(word_obj.begin(), word_obj.end());
            return static_cast<uint8_t>(convert_input(word, 0, max_val));
        }) |
        std::ranges::to<std::vector>();

    if (len != converted_input.size()) {
        exit_with_error();
    }

    return converted_input;
}

void output_vector(std::vector<uint8_t> &out)
{
    for (size_t i = 0; i < out.size(); ++i) {
        if (i > 0) {
            std::cout << " ";
        }
        std::cout << static_cast<int>(out[i]);
    }
    std::cout << std::endl;
}

} // namespace

// The function implements the codemaker role in the Mastermind game.
// It reads the player's attempts and responds with the number of hits on correct positions
// and the number of hits on wrong positions, until the sequence is guessed.
void codemaker(size_t num_colors, std::vector<uint8_t> &secret_sequence)
{
    size_t sequence_len = secret_sequence.size();
    uint8_t position_matches = 0, value_matches = 0;
    while (position_matches != sequence_len or value_matches != 0) {
        std::vector<uint8_t> query = input_vector(sequence_len, num_colors - 1);

        std::unordered_map<uint8_t, uint8_t> count_seq, count_que, count_same;
        std::set<uint8_t> values;

        for (uint8_t &x : secret_sequence) {
            ++count_seq[x], values.emplace(x);
        }
        for (uint8_t &x : query) {
            ++count_que[x];
        }

        std::vector<uint8_t> same_positions =
            std::views::iota(0, static_cast<int>(sequence_len)) |
            std::views::transform([&](uint8_t i) {
                return static_cast<uint8_t>(secret_sequence[i] == query[i]);
            }) |
            std::ranges::to<std::vector>();

        for (size_t i = 0; i < sequence_len; ++i) {
            count_same[secret_sequence[i]] += same_positions[i];
        }

        position_matches = std::accumulate(same_positions.begin(),
            same_positions.end(),
            static_cast<uint8_t>(0));

        value_matches = std::accumulate(values.begin(),
            values.end(),
            static_cast<uint8_t>(0),
            [&](uint8_t acc, uint8_t x) {
                uint8_t min_occurenes = std::min(count_seq[x], count_que[x]);
                return acc + min_occurenes - count_same[x];
            });

        std::cout << static_cast<int>(position_matches) << " "
                  << static_cast<int>(value_matches) << std::endl;
    }
}

// The function implements the codebreaker in the Mastermind game.
// It uses the following strategy to guess the sequence:
// 1. Determines the number of occurrences of each color by querying homogeneous
// sequences
// 2. Determines the color at each position by querying each of them with
// frozen values at other positions
void codebreaker(size_t num_colors, size_t sequence_len)
{
    auto final_guess = [&](std::vector<uint8_t> &result) {
        output_vector(result);
        std::vector<uint8_t> final_response = input_vector(2, sequence_len);
        if (final_response[0] != sequence_len or final_response[1] != 0) {
            exit_with_error();
        }
    };

    uint16_t total_count = 0;

    std::vector<uint8_t> counts =
        std::views::iota(0, static_cast<int>(num_colors)) |
        std::views::transform([&](uint8_t i) {
            std::vector<uint8_t> query(sequence_len, i);
            output_vector(query);

            std::vector<uint8_t> input = input_vector(2, sequence_len);
            auto [position_matches, value_matches] =
                std::make_tuple(input[0], input[1]);
            total_count += position_matches;

            if (value_matches != 0 or sequence_len < total_count) {
                exit_with_error();
            }
            if (position_matches == sequence_len) {
                throw NormalExitException();
            }
            return position_matches;
        }) |
        std::ranges::to<std::vector>();

    if (total_count != sequence_len) {
        exit_with_error();
    }

    uint8_t max_present = 0;
    std::vector<uint8_t> present_values;

    for (size_t i = 0; i < counts.size(); ++i) {
        if (counts[i]) {
            present_values.emplace_back(i);
            max_present = i;
        }
    }
    // We remove the last one to avoid repeating the query for the full sequence
    present_values.pop_back();

    std::vector<uint8_t> result = std::vector<uint8_t>(sequence_len);

    // Logic for guessing the color at position i:
    // - If position_matches increased by 1: the color occurs at this position
    // - If value_matches is 1: the color belongs to the sequence, but at a different
    //   position
    // - If position_matches decreased by 1 and value_matches is 2 at the
    //   first attempt: the max_present color occurs at this position
    for (size_t i = 0; i < sequence_len; ++i) {
        bool first_guess = true;
        bool guessed = false;
        for (uint8_t value : present_values) {
            std::vector<uint8_t> query(sequence_len, max_present);
            query[i] = value;
            output_vector(query);

            std::vector<uint8_t> input = input_vector(2, sequence_len);
            auto [position_matches, value_matches] =
                std::make_tuple(input[0], input[1]);

            if (position_matches == counts[max_present] + 1 and
                value_matches == 0) {
                result[i] = value;
                guessed = true;
            }
            else if (position_matches == counts[max_present] and
                     value_matches == 1) {
                first_guess = false;
            }
            else if (position_matches == counts[max_present] - 1 and
                     value_matches == 2 and first_guess) {
                result[i] = max_present;
                guessed = true;
            }
            else {
                exit_with_error();
            }

            if (guessed) {
                if (position_matches == sequence_len and value_matches == 0) {
                    throw NormalExitException();
                }
                else {
                    break;
                }
            }

            first_guess = false;
        }
        if (!guessed) {
            exit_with_error();
        }
    }

    std::vector<uint8_t> real_counts(num_colors, 0);
    for (uint8_t value : result) {
        ++real_counts[value];
    }
    if (counts != real_counts) {
        exit_with_error();
    }

    final_guess(result);
}

int main(int argc, char **argv)
{
    try {
        auto check_kn_bounds = [](size_t num_colors, size_t sequence_len) {
            uint64_t value = 1, bound = MAX_COMBINATIONS;
            while (sequence_len and value <= bound) {
                value *= num_colors, --sequence_len;
            }
            if (sequence_len or bound < value) {
                exit_with_error();
            }
        };
        if (argc <= 2) {
            exit_with_error();
        }
        else if (argc == 3) {
            size_t num_colors =
                convert_input(argv[1], MIN_NUM_COLORS, MAX_NUM_COLORS);
            size_t sequence_len =
                convert_input(argv[2], MIN_SEQUENCE_LEN, MAX_SEQUENCE_LEN);
            check_kn_bounds(num_colors, sequence_len);
            codebreaker(num_colors, sequence_len);
        }
        else {
            size_t num_colors =
                convert_input(argv[1], MIN_NUM_COLORS, MAX_NUM_COLORS);
            std::vector<uint8_t> secret_sequence;

            for (int i = 2; i < argc; ++i) {
                size_t value = convert_input(argv[i], 0, num_colors - 1);
                secret_sequence.emplace_back(static_cast<uint8_t>(value));
            }

            if (secret_sequence.size() < MIN_SEQUENCE_LEN
                or secret_sequence.size() > MAX_SEQUENCE_LEN) {
                exit_with_error();
            }

            check_kn_bounds(num_colors, secret_sequence.size());
            codemaker(num_colors, secret_sequence);
        }
    }
    catch (const NormalExitException &) {
        return 0;
    }

    return 0;
}