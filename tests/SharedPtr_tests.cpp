#include <gtest/gtest.h>
#include "SharedPtr.hpp"
#include "mutable_array_sequence.hpp"
#include <utility>

struct LeakTracker {
    static inline int alive_count = 0;

    LeakTracker() { alive_count++; }
    virtual ~LeakTracker() { alive_count--; }
};

struct DerivedLeakTracker : public LeakTracker {
    static inline int derived_alive_count = 0;

    DerivedLeakTracker() { derived_alive_count++; }
    ~DerivedLeakTracker() override { derived_alive_count--; }
};

TEST(SharedPtrTests, EmptyPointerTests) {
    SharedPtr<Sequence<int>> sp;
    EXPECT_EQ(sp.get(), nullptr);
    EXPECT_EQ(sp.get_count(), 0);

    if (sp) {
        FAIL();
    } else {
        SUCCEED();
    }
}

TEST(SharedPtrTests, ArrowOperator) {
    SharedPtr<MutableArraySequence<int>> sp(new MutableArraySequence<int>());

    EXPECT_NE(sp.get(), nullptr);
    EXPECT_EQ(sp.get_count(), 1);
    if (sp) {
        SUCCEED();
    } else {
        FAIL();
    }

    sp->append(10);
    sp->append(20);
    sp->append(30);

    EXPECT_EQ(sp->get_first(), 10);
    EXPECT_EQ(sp->get_last(), 30);
    EXPECT_EQ(sp->get_size(), 3);
}

TEST(SharedPtrTests, DereferenceOperatorTests) {
    SharedPtr<MutableArraySequence<int>> sp(new MutableArraySequence<int>());

    sp->append(10);
    sp->append(2313);
    sp->append(532542);

    MutableArraySequence<int>& seq = *sp;

    EXPECT_EQ(seq.get_size(), 3);
    EXPECT_EQ(seq.get_first(), 10);
}

TEST(SharedPtrTests, EqualityOperators) {
    SharedPtr<MutableArraySequence<int>> sp1(new MutableArraySequence<int>());
    SharedPtr<MutableArraySequence<int>> sp2 = sp1;
    SharedPtr<MutableArraySequence<int>> sp3(new MutableArraySequence<int>());
    SharedPtr<MutableArraySequence<int>> sp_empty;

    EXPECT_TRUE(sp1 == sp2);
    EXPECT_FALSE(sp1 != sp2);

    EXPECT_FALSE(sp1 == sp3);
    EXPECT_TRUE(sp1 != sp3);

    EXPECT_FALSE(sp1 == sp_empty);
    EXPECT_TRUE(sp1 != sp_empty);
}

TEST(SharedPtrTests, CopyConstuctorTest) {
    SharedPtr<MutableArraySequence<int>> sp1(new MutableArraySequence<int>());
    sp1->append(10);
    EXPECT_EQ(sp1.get_count(), 1);
    EXPECT_EQ(sp1->get_first(), 10);

    {
        SharedPtr<MutableArraySequence<int>> sp2(sp1);
        sp2->append(20);
        EXPECT_EQ(sp1.get(), sp2.get());
        EXPECT_EQ(sp1.get_count(), 2);
        EXPECT_EQ(sp2.get_count(), 2);
        EXPECT_EQ(sp2->get_last(), 20);
    }

    EXPECT_EQ(sp1->get_last(), 20);
    EXPECT_EQ(sp1.get_count(), 1);
}

TEST(SharedPtrTests, EqualOperator) {
    SharedPtr<MutableArraySequence<int>> sp1(new MutableArraySequence<int>());
    sp1->append(100);

    SharedPtr<MutableArraySequence<int>> sp2(new MutableArraySequence<int>());
    sp2->append(200);

    EXPECT_EQ(sp1.get_count(), 1);
    EXPECT_EQ(sp2.get_count(), 1);

    sp2 = sp1;

    EXPECT_EQ(sp1.get_count(), 2);
    EXPECT_EQ(sp2.get_count(), 2);
    EXPECT_EQ(sp1.get(), sp2.get());
    EXPECT_EQ(sp2->get_first(), 100);

    sp1 = sp1;

    EXPECT_EQ(sp1.get_count(), 2);
    EXPECT_EQ(sp1->get_first(), 100);
}

TEST(SharedPtrTests, MoveTests) {
    SharedPtr<MutableArraySequence<int>> sp1(new MutableArraySequence<int>());
    sp1->append(42);

    EXPECT_EQ(sp1.get_count(), 1);

    SharedPtr<MutableArraySequence<int>> sp2(std::move(sp1));

    EXPECT_EQ(sp1.get(), nullptr);
    EXPECT_EQ(sp1.get_count(), 0);

    EXPECT_NE(sp2.get(), nullptr);
    EXPECT_EQ(sp2.get_count(), 1);
    EXPECT_EQ(sp2->get_first(), 42);

    SharedPtr<MutableArraySequence<int>> sp3(new MutableArraySequence<int>());
    sp3->append(999);

    sp3 = std::move(sp2);

    EXPECT_EQ(sp2.get(), nullptr);
    EXPECT_EQ(sp2.get_count(), 0);

    EXPECT_NE(sp3.get(), nullptr);
    EXPECT_EQ(sp3.get_count(), 1);
    EXPECT_EQ(sp3->get_first(), 42);
}

TEST(SharedPtrTests, ResetTests) {
    SharedPtr<MutableArraySequence<int>> sp1(new MutableArraySequence<int>());
    sp1->append(10);
    SharedPtr<MutableArraySequence<int>> sp2 = sp1;

    EXPECT_EQ(sp1.get_count(), 2);

    sp1.reset();

    EXPECT_EQ(sp1.get(), nullptr);
    EXPECT_EQ(sp1.get_count(), 0);
    EXPECT_EQ(sp2.get_count(), 1);
    EXPECT_EQ(sp2->get_first(), 10);

    sp2.reset();

    EXPECT_EQ(sp2.get(), nullptr);
    EXPECT_EQ(sp2.get_count(), 0);

    SharedPtr<MutableArraySequence<int>> sp3(new MutableArraySequence<int>());
    sp3->append(100);

    MutableArraySequence<int>* new_seq = new MutableArraySequence<int>();
    new_seq->append(200);
    sp3.reset(new_seq);

    EXPECT_EQ(sp3.get_count(), 1);
    EXPECT_EQ(sp3->get_first(), 200);
}

TEST(SharedPtrSubtyping, PolymorphicCopyUpcast) {
    SharedPtr<MutableArraySequence<int>> derived(new MutableArraySequence<int>());
    derived->append(10);
    derived->append(20);

    EXPECT_EQ(derived.get_count(), 1);

    {
        SharedPtr<Sequence<int>> base = derived;

        EXPECT_EQ(derived.get_count(), 2);
        EXPECT_EQ(base.get_count(), 2);

        EXPECT_EQ(base->get_size(), 2);
        EXPECT_EQ(base->get_first(), 10);
        EXPECT_EQ(base->get_last(), 20);

        derived->append(30);
        EXPECT_EQ(base->get_size(), 3);
        EXPECT_EQ(base->get_last(), 30);
    }

    EXPECT_EQ(derived.get_count(), 1);
    EXPECT_EQ(derived->get_size(), 3);
}

TEST(SharedPtrSubtyping, PolymorphicMoveTest) {
    SharedPtr<MutableArraySequence<int>> derived(new MutableArraySequence<int>());
    derived->append(777);

    EXPECT_EQ(derived.get_count(), 1);

    SharedPtr<Sequence<int>> base = std::move(derived);

    EXPECT_EQ(derived.get(), nullptr);
    EXPECT_EQ(derived.get_count(), 0);

    EXPECT_NE(base.get(), nullptr);
    EXPECT_EQ(base.get_count(), 1);
    EXPECT_EQ(base->get_size(), 1);
    EXPECT_EQ(base->get_first(), 777);
}


TEST(SharedPtrMemoryLeaks, LifetimeAndScopeDestruction) {
    LeakTracker::alive_count = 0;

    {
        SharedPtr<LeakTracker> sp1(new LeakTracker());
        EXPECT_EQ(LeakTracker::alive_count, 1);

        {
            SharedPtr<LeakTracker> sp2 = sp1;
            SharedPtr<LeakTracker> sp3 = sp2;
            EXPECT_EQ(LeakTracker::alive_count, 1);
            EXPECT_EQ(sp1.get_count(), 3);
        }

        EXPECT_EQ(LeakTracker::alive_count, 1);
        EXPECT_EQ(sp1.get_count(), 1);
    }

    EXPECT_EQ(LeakTracker::alive_count, 0);
}

TEST(SharedPtrMemoryLeaks, ResetDestruction) {
    LeakTracker::alive_count = 0;

    SharedPtr<LeakTracker> sp(new LeakTracker());
    EXPECT_EQ(LeakTracker::alive_count, 1);

    sp.reset();

    EXPECT_EQ(LeakTracker::alive_count, 0);
    EXPECT_EQ(sp.get_count(), 0);
}

TEST(SharedPtrMemoryLeaks, PolymorphicDestruction) {
    LeakTracker::alive_count = 0;
    DerivedLeakTracker::derived_alive_count = 0;

    {
        SharedPtr<DerivedLeakTracker> derived(new DerivedLeakTracker());
        EXPECT_EQ(LeakTracker::alive_count, 1);
        EXPECT_EQ(DerivedLeakTracker::derived_alive_count, 1);

        SharedPtr<LeakTracker> base = std::move(derived);
        EXPECT_EQ(LeakTracker::alive_count, 1);
        EXPECT_EQ(DerivedLeakTracker::derived_alive_count, 1);
    }

    EXPECT_EQ(DerivedLeakTracker::derived_alive_count, 0);
    EXPECT_EQ(LeakTracker::alive_count, 0);
}

