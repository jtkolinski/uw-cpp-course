export module MothBase;
export import TextBase;

export import <cstddef>;
export import <ostream>; 

export class MothBase {
protected:
    TextBase& text;
    size_t position;
    size_t vitality;
    size_t P;
    bool active;
    char type_char;

    // Moth penalites for moving and consuming a space
    static constexpr std::size_t SPACE_PENALTY = 32;
    static constexpr std::size_t MOVE_PENALTY = 10;

    virtual bool eating_criterion(char c) const noexcept = 0;
    virtual bool change_position() noexcept = 0;

public:
    MothBase(TextBase& _text, size_t _position, size_t _vitality , size_t _p, char _type_char): 
        text(_text), position(_position), vitality(_vitality), P(_p),
        active(true), type_char(_type_char) {
            // Moth dies immediately when spawned out of bounds
            if (!text.is_valid_position(position)) {
                active = false;
                vitality = 0;
            }
        }

    virtual ~MothBase() = default;

    virtual void feeding_cycle() = 0;

    virtual bool is_active() const noexcept = 0;

    friend std::ostream& operator <<(std::ostream &os, const MothBase& moth) {
        os << moth.type_char << " "
           << moth.P << " " 
           << moth.position << " "
           << moth.vitality;
        return os;
    }
};