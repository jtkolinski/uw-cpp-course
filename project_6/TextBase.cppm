export module TextBase;

export import <cstddef>;

export class TextBase {
public:
    virtual ~TextBase() = default;
    virtual bool is_valid_position(size_t position) const noexcept = 0;
    virtual const char& get_char(size_t position) const noexcept = 0;
    virtual char& get_char(size_t position) noexcept = 0;
    virtual size_t get_new_pos(size_t curr_pos, size_t step_length) const noexcept = 0;
};