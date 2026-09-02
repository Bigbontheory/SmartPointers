#include "bit_sequence.hpp"

BitSequence::BitSequence() : data() {}

BitSequence::BitSequence(bool* items, int size) {
    data.resize(size);
    for (int i = 0; i < size; ++i) {
        data.set(i, Bit(items[i]));
    }
}

BitSequence::BitSequence(const BitSequence& other) : data(other.data) {}

BitSequence* BitSequence::clone() const {
    return new BitSequence(*this);
}

ISequenceBuilder<Bit>* BitSequence::create_builder() const {
    return new SequenceBuilder<Bit, BitSequence>();
}

const Bit& BitSequence::get_first() const {
    return data.get(0);
}

const Bit& BitSequence::get_last() const {
    return data.get(data.get_size() - 1);
}

const Bit& BitSequence::get(int index) const {
    return data.get(index);
}

int BitSequence::get_size() const {
    return data.get_size();
}

BitSequence* BitSequence::append(const Bit& item) {
    append_internal(item);
    return this;
}

BitSequence* BitSequence::prepend(const Bit& item) {
    int old_size = data.get_size();
    data.resize(old_size + 1);
    for (int i = old_size; i > 0; --i) {
        data.set(i, data.get(i - 1));
    }
    data.set(0, item);
    return this;
}

BitSequence* BitSequence::insert_at(const Bit& item, int index) {
    int old_size = data.get_size();
    data.resize(old_size + 1);
    for (int i = old_size; i > index; --i) {
        data.set(i, data.get(i - 1));
    }
    data.set(index, item);
    return this;
}

BitSequence* BitSequence::remove_at(int index) {
    data.remove_at(index);
    return this;
}

BitSequence* BitSequence::get_subsequence(int start_index, int end_index) const {
    BitSequence* res = new BitSequence();
    for (int i = start_index; i <= end_index; ++i) {
        res->append(this->get(i));
    }
    return res;
}

BitSequence* BitSequence::concat(const Sequence<Bit>* other) const {
    BitSequence* res = new BitSequence(*this);
    IEnumerator<Bit>* it = other->get_enumerator();
    while (it->move_next()) {
        res->append(it->get_current());
    }
    delete it;
    return res;
}

BitSequence* BitSequence::map(Bit(*mapper)(const Bit& element)) const {
    BitSequence* res = new BitSequence();
    for (int i = 0; i < this->get_size(); ++i) {
        res->append(mapper(this->get(i)));
    }
    return res;
}

BitSequence* BitSequence::where(bool (*predicate)(const Bit& element)) const {
    BitSequence* res = new BitSequence();
    for (int i = 0; i < this->get_size(); ++i) {
        if (predicate(this->get(i))) {
            res->append(this->get(i));
        }
    }
    return res;
}

Bit BitSequence::reduce(Bit(*reduce_func)(const Bit& first, const Bit& second), const Bit& start_element) const {
    Bit res = start_element;
    for (int i = 0; i < this->get_size(); ++i) {
        res = reduce_func(res, this->get(i));
    }
    return res;
}

BitSequence* BitSequence::bit_and(const BitSequence& other) const {
    BitSequence* res = new BitSequence();
    int min_size = this->get_size() < other.get_size() ? this->get_size() : other.get_size();
    for (int i = 0; i < min_size; ++i) {
        res->append(this->get(i) & other.get(i));
    }
    return res;
}

BitSequence* BitSequence::bit_or(const BitSequence& other) const {
    BitSequence* res = new BitSequence();
    int min_size = this->get_size() < other.get_size() ? this->get_size() : other.get_size();
    for (int i = 0; i < min_size; ++i) {
        res->append(this->get(i) | other.get(i));
    }
    return res;
}

BitSequence* BitSequence::bit_xor(const BitSequence& other) const {
    BitSequence* res = new BitSequence();
    int min_size = this->get_size() < other.get_size() ? this->get_size() : other.get_size();
    for (int i = 0; i < min_size; ++i) {
        res->append(this->get(i) ^ other.get(i));
    }
    return res;
}

BitSequence* BitSequence::bit_not() const {
    BitSequence* res = new BitSequence();
    for (int i = 0; i < this->get_size(); ++i) {
        res->append(~this->get(i));
    }
    return res;
}

BitSequence* BitSequence::slice(int index, int count, const Sequence<Bit>& seq) {
    for (int i = 0; i < count; ++i) {
        if (index < this->get_size()) {
            this->remove_at(index);
        }
    }
    IEnumerator<Bit>* it = seq.get_enumerator();
    int curr_idx = index;
    while (it->move_next()) {
        this->insert_at(it->get_current(), curr_idx++);
    }
    delete it;
    return this;
}

IEnumerator<Bit>* BitSequence::get_enumerator() const {
    return new BitEnumerator(this);
}

bool BitSequence::BitEnumerator::move_next() {
    if (index + 1 < bit_sequence->get_size()) {
        index++;
        return true;
    }
    return false;
}

const Bit& BitSequence::BitEnumerator::get_current() const {
    return bit_sequence->get(index);
}

void BitSequence::BitEnumerator::reset() {
    index = -1;
}