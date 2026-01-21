import Colony;
import Moths;

import <iostream>;
import <regex>;
import <memory>;
import <string>;
import <exception>;

int main() {
    Colony colony;

    const std::regex re_text(R"(^TEXT (0|[1-9]\d*) ([!-~]+)$)");
    const std::regex re_moth(
        R"(^MOTH (0|[1-9]\d*) (0|[1-9]\d*) ([*1A!]) (0|[1-9]\d*) ([1-9][0-9]?)$)"
    );
    const std::regex re_feed(R"(^FEED (0|[1-9]\d*) (0|[1-9]\d*)$)");
    const std::regex re_printm(R"(^PRINTM (0|[1-9]\d*)$)");
    const std::regex re_printt(R"(^PRINTT (0|[1-9]\d*)$)");
    const std::regex re_delete(R"(^DELETE (0|[1-9]\d*)$)");

    std::string input;
    size_t line_counter = 0;

    auto print_error = [](size_t line_num, std::ostream& os = std::cerr) {
        os << "ERROR " << line_num << "\n";
    };

    while (std::getline(std::cin, input)) {
        ++line_counter;
        std::smatch matches;

        try {
            if (std::regex_match(input, matches, re_text)) {
                size_t t = std::stoull(matches[1].str());
                std::string text = matches[2].str();

                if (colony.contains(t)) print_error(line_counter);
                else colony[t] = Text(text);
            } else if (std::regex_match(input, matches, re_moth)) {
                size_t t = std::stoull(matches[1].str());
                size_t n = std::stoull(matches[2].str());
                char r = matches[3].str()[0];
                size_t v = std::stoull(matches[4].str());
                size_t p = std::stoull(matches[5].str());

                if (!colony.contains(t) || !colony[t].is_valid_position(n)) {
                    print_error(line_counter);
                } else {
                    std::unique_ptr<Moth> moth_ptr;
                    switch (r) {
                        case '1':
                            moth_ptr = std::make_unique<NumberMoth>(colony[t], n, v, p, r);
                            break;
                        case 'A':
                            moth_ptr = std::make_unique<LetterMoth>(colony[t], n, v, p, r);
                            break;
                        case '!':
                            moth_ptr = std::make_unique<PickyMoth>(colony[t], n, v, p, r);
                            break;
                        default:
                            moth_ptr = std::make_unique<CommonMoth>(colony[t], n, v, p, r);
                    }

                    colony[t].add_moth(std::move(moth_ptr));
                }
            } else if (std::regex_match(input, matches, re_feed)) {
                size_t t = std::stoull(matches[1].str());
                size_t c = std::stoull(matches[2].str());

                if (!colony.contains(t)) print_error(line_counter);
                else colony[t].feeding_cycles(c);
            } else if (std::regex_match(input, matches, re_printm)) {
                size_t t = std::stoull(matches[1].str());

                if (!colony.contains(t)) print_error(line_counter);
                else colony[t].print_moths();
            } else if (std::regex_match(input, matches, re_printt)) {
                size_t t = std::stoull(matches[1].str());

                if (!colony.contains(t)) print_error(line_counter);
                else std::cout << colony[t];
            } else if (std::regex_match(input, matches, re_delete)) {
                size_t t = std::stoull(matches[1].str());

                if (!colony.contains(t)) print_error(line_counter);
                else colony.erase(t);
            } else {
                print_error(line_counter, std::cout);
            }

        } catch (std::exception e) {
            print_error(line_counter);
        }
    }

    return 0;
}