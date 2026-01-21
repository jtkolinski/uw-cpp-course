export module Text;

export import TextBase;
export import MothBase;

export import <string>;
export import <memory>;
export import <iostream>;
export import <vector>;
export import <utility>;
export import <cstddef>;

export class Text : public TextBase {
private:
    std::string text;
    std::vector<std::unique_ptr<MothBase>> moths;
public:
    Text() = default;

    explicit Text(std::string _text): text(std::move(_text)) {}

    Text(Text&&) noexcept = default;
    Text& operator=(Text&&) noexcept = default;

    ~Text() override = default;

    bool is_valid_position(size_t position) const noexcept override {
        return position < text.length();
    }

    const char& get_char(size_t position) const noexcept override {
        return text[position];
    }

    char& get_char(size_t position) noexcept override {
        return text[position];
    }

    size_t get_new_pos(size_t curr_pos, size_t step_length) const noexcept override {
        if (text.empty()) return 0;
        return (curr_pos + step_length) % text.length();
    }

    void add_moth(std::unique_ptr<MothBase> ptr) {
        if (ptr) moths.emplace_back(std::move(ptr));
    }

    void feeding_cycles(size_t num_cycles) {
        for (size_t cycle = 0; cycle < num_cycles; ++cycle) {
            for (const auto& moth : moths) {
                if (!moth->is_active()) continue;
                moth->feeding_cycle();
            }
        }
    }

    void print_moths(std::ostream& os = std::cout) const {
        for (const auto& moth : moths)
            os << *moth << std::endl;
    }

    friend std::ostream& operator <<(std::ostream &os, const Text& _text) {
        os << _text.text << std::endl;
        return os;
    }
};