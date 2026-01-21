export module Moths;

export import Moth;

export import <cstddef>;

export class FixedStepMoth : public Moth {
protected:
    virtual bool change_position() noexcept override {
        size_t cost = P * MOVE_PENALTY;

        if (vitality < cost) {
            vitality = 0;
            active = false;
        } else {
            vitality -= cost;
            position = text.get_new_pos(position, P);
        }
        return active;
    }
public:
    using Moth::Moth;
};

export class CommonMoth : public FixedStepMoth {
private:
    bool eating_criterion(char) const noexcept override {
        return true;
    }
public:
    using FixedStepMoth::FixedStepMoth;
};

export class LetterMoth : public FixedStepMoth {
private:
    bool eating_criterion(char c) const noexcept override {
        // cast to unsigned char to prevent UB
        return std::isalpha(static_cast<unsigned char>(c)); 
    }
public:
    using FixedStepMoth::FixedStepMoth;
};

export class NumberMoth : public FixedStepMoth {
private:
    bool eating_criterion(char c) const noexcept override {
        // cast to unsigned char to prevent UB
        return std::isdigit(static_cast<unsigned char>(c));
    }
public:
    using FixedStepMoth::FixedStepMoth;
};

export class PickyMoth : public Moth {
private:
    size_t step_length = 1;

    bool change_position() noexcept override {
        size_t cost = step_length * MOVE_PENALTY;

        if (vitality < cost) {
            vitality = 0;
            active = false;
        } else {
            position = text.get_new_pos(position, step_length);
            vitality -= cost;
        }

        step_length = step_length % P + 1;
        return active;
    }

    bool eating_criterion(char c) const noexcept override {
        // cast to unsigned char to prevent UB
        return !std::isalnum(static_cast<unsigned char>(c));
    }
public:
    using Moth::Moth;
};