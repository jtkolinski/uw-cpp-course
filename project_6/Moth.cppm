export module Moth;

export import MothBase;

export class Moth : public MothBase {
public:
    using MothBase::MothBase;

    ~Moth() override = default;

    // Attempts to move to designated position and to eat a character located there
    void feeding_cycle() override {
        if (!active || !change_position()) return;

        char &char_to_eat = text.get_char(position);
        if (char_to_eat == ' ') {
            vitality = vitality >= SPACE_PENALTY ? vitality - SPACE_PENALTY : 0;
        } else if (eating_criterion(char_to_eat)) {
            // char can be negative so we need to cast it to unsigned char
            // to prevent overflows
            auto value = static_cast<std::size_t>(static_cast<unsigned char>(char_to_eat));
            vitality += value;
            char_to_eat = ' ';
        }
    }

    bool is_active() const noexcept override { return active; }
};