#pragma once

#include "bit.hpp"
#include "dynamicarray.hpp"
#include "sequence.hpp"


template <typename T> class ISequenceBuilder;

class BitSequence : public Sequence<Bit> {
public:

    BitSequence();
    BitSequence(bool* items, int size);
    BitSequence(const BitSequence& other);
    virtual ~BitSequence() = default;

    BitSequence* clone() const override;
    ISequenceBuilder<Bit>* create_builder() const override;

    const Bit& get_first() const override;
    const Bit& get_last() const override;
    const Bit& get(int index) const override;
    int get_size() const override;

    BitSequence* append(const Bit& item) override;
    BitSequence* prepend(const Bit& item) override;
    BitSequence* insert_at(const Bit& item, int index) override;
    BitSequence* remove_at(int index) override;
    BitSequence* get_subsequence(int start_index, int end_index) const override;

    BitSequence* set(const Bit& item, int index);

    BitSequence* concat(const Sequence<Bit>* other) const override;
    BitSequence* map(Bit(*mapper)(const Bit& element)) const override;
    BitSequence* where(bool (*predicate)(const Bit& element)) const override;
    Bit reduce(Bit(*reduce_func)(const Bit& first_element, const Bit& second_element), const Bit& start_element) const override;

    BitSequence* bit_and(const BitSequence& other) const;
    BitSequence* bit_or(const BitSequence& other) const;
    BitSequence* bit_xor(const BitSequence& other) const;
    BitSequence* bit_not() const;
    BitSequence* slice(int index, int count, const Sequence<Bit>& seq);


    class BitEnumerator : public IEnumerator<Bit> {
    public:
        BitEnumerator(const BitSequence* bit_sequence) : bit_sequence{ bit_sequence }, index{ -1 } {}

        bool move_next() override;
        const Bit& get_current() const override;
        void reset() override;

    private:
        int index;
        const BitSequence* bit_sequence;
    };


    IEnumerator<Bit>* get_enumerator() const override;

protected:
    template <typename T, typename SeqType> friend class SequenceBuilder;
    void append_internal(const Bit& item) {
        int sz = data.get_size();
        data.resize(sz + 1);
        data.set(sz, item);
    }

private:
    DynamicArray<Bit> data;
};