#pragma once

#include <ostream>
class Bit {
public:
    Bit() : value{ false } {}
    Bit(bool value) : value{ value } {}
    Bit(int value) : value{ value != 0 } {}

    Bit(const Bit& other) {
        this->value = other.value;
    }

    Bit& operator=(const Bit& other) {
        if (this == &other)
            return *this;
        this->value = other.value;
        return *this;
    }

    bool get() const {
        return value;
    }

    Bit operator&(const Bit& other) const {
        return Bit(value && other.value);
    }

    Bit operator|(const Bit& other) const {
        return Bit(value || other.value);
    }

    Bit operator^(const Bit& other) const {
        return Bit(value != other.value);
    }

    Bit operator~() const {
        return Bit(!value);
    }

    bool operator==(const Bit& other) const {
        return value == other.value;
    }

    bool operator!=(const Bit& other) const {
        return value != other.value;
    }

private:
    bool value;
};

inline std::ostream& operator<<(std::ostream& os, const Bit& bit) {
    return os << (bit.get() ? "1" : "0");
}