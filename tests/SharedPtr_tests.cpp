#include <gtest/gtest.h>
#include <utility>
#include "SharedPtr.hpp"

struct LeakTracker {
    static inline int alive_count = 0;

    int value{0};

    LeakTracker() { ++alive_count; }
    explicit LeakTracker(int v) : value(v) { ++alive_count; }
    virtual ~LeakTracker() { --alive_count; }
};

struct DerivedLeakTracker : public LeakTracker {
    static inline int derived_alive_count = 0;

    DerivedLeakTracker() : LeakTracker() { ++derived_alive_count; }
    explicit DerivedLeakTracker(int v) : LeakTracker(v) { ++derived_alive_count; }
    ~DerivedLeakTracker() override { --derived_alive_count; }
};

void delete_leak_tracker_array(LeakTracker* p) {
    delete[] p;
}

void delete_derived_leak_tracker_array(DerivedLeakTracker* p) {
    delete[] p;
}

struct CustomFunctorDeleter {
    static inline int calls_count = 0;

    void operator()(LeakTracker* p) const {
        ++calls_count;
        delete p;
    }
};

TEST(SharedPtrTests, EmptyPointerTests) {
    SharedPtr<LeakTracker> sp;
    EXPECT_EQ(sp.get(), nullptr);
    EXPECT_EQ(sp.use_count(), 0);
    EXPECT_FALSE(static_cast<bool>(sp));
}

TEST(SharedPtrTests, ArrowAndDereferenceOperators) {
    SharedPtr<LeakTracker> sp(new LeakTracker(42));

    EXPECT_NE(sp.get(), nullptr);
    EXPECT_EQ(sp.use_count(), 1);
    EXPECT_TRUE(static_cast<bool>(sp));

    EXPECT_EQ(sp->value, 42);
    EXPECT_EQ((*sp).value, 42);

    sp->value = 100;
    EXPECT_EQ(sp->value, 100);
}

TEST(SharedPtrTests, CopyConstructorAndAssignment) {
    SharedPtr<LeakTracker> sp1(new LeakTracker(10));
    EXPECT_EQ(sp1.use_count(), 1);

    {
        SharedPtr<LeakTracker> sp2(sp1);
        EXPECT_EQ(sp1.get(), sp2.get());
        EXPECT_EQ(sp1.use_count(), 2);
        EXPECT_EQ(sp2.use_count(), 2);
    }

    EXPECT_EQ(sp1.use_count(), 1);

    SharedPtr<LeakTracker> sp3(new LeakTracker(20));
    sp3 = sp1;

    EXPECT_EQ(sp1.use_count(), 2);
    EXPECT_EQ(sp3.use_count(), 2);
    EXPECT_EQ(sp3->value, 10);

    sp1 = sp1;
    EXPECT_EQ(sp1.use_count(), 2);
}

TEST(SharedPtrTests, MoveConstructorAndAssignment) {
    SharedPtr<LeakTracker> sp1(new LeakTracker(42));
    EXPECT_EQ(sp1.use_count(), 1);

    SharedPtr<LeakTracker> sp2(std::move(sp1));
    EXPECT_EQ(sp1.get(), nullptr);
    EXPECT_EQ(sp1.use_count(), 0);
    EXPECT_NE(sp2.get(), nullptr);
    EXPECT_EQ(sp2.use_count(), 1);
    EXPECT_EQ(sp2->value, 42);

    SharedPtr<LeakTracker> sp3(new LeakTracker(999));
    sp3 = std::move(sp2);

    EXPECT_EQ(sp2.get(), nullptr);
    EXPECT_EQ(sp2.use_count(), 0);
    EXPECT_NE(sp3.get(), nullptr);
    EXPECT_EQ(sp3.use_count(), 1);
    EXPECT_EQ(sp3->value, 42);
}

TEST(SharedPtrTests, ResetTests) {
    SharedPtr<LeakTracker> sp1(new LeakTracker(10));
    SharedPtr<LeakTracker> sp2 = sp1;

    EXPECT_EQ(sp1.use_count(), 2);

    sp1.reset();
    EXPECT_EQ(sp1.get(), nullptr);
    EXPECT_EQ(sp1.use_count(), 0);
    EXPECT_EQ(sp2.use_count(), 1);
    EXPECT_EQ(sp2->value, 10);

    sp2.reset(new LeakTracker(200));
    EXPECT_EQ(sp2.use_count(), 1);
    EXPECT_EQ(sp2->value, 200);
}

TEST(SharedPtrSubtyping, PolymorphicCopyUpcast) {
    SharedPtr<DerivedLeakTracker> derived(new DerivedLeakTracker(777));
    EXPECT_EQ(derived.use_count(), 1);

    {
        SharedPtr<LeakTracker> base = derived;

        EXPECT_EQ(derived.use_count(), 2);
        EXPECT_EQ(base.use_count(), 2);
        EXPECT_EQ(base->value, 777);

        derived->value = 888;
        EXPECT_EQ(base->value, 888);
    }

    EXPECT_EQ(derived.use_count(), 1);
    EXPECT_EQ(derived->value, 888);
}

TEST(SharedPtrSubtyping, PolymorphicMoveTest) {
    SharedPtr<DerivedLeakTracker> derived(new DerivedLeakTracker(777));
    EXPECT_EQ(derived.use_count(), 1);

    SharedPtr<LeakTracker> base = std::move(derived);

    EXPECT_EQ(derived.get(), nullptr);
    EXPECT_EQ(derived.use_count(), 0);

    EXPECT_NE(base.get(), nullptr);
    EXPECT_EQ(base.use_count(), 1);
    EXPECT_EQ(base->value, 777);
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
            EXPECT_EQ(sp1.use_count(), 3);
        }

        EXPECT_EQ(LeakTracker::alive_count, 1);
        EXPECT_EQ(sp1.use_count(), 1);
    }

    EXPECT_EQ(LeakTracker::alive_count, 0);
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

TEST(SharedPtrDeleterTests, SingleObjectArrayDeleterFunction) {
    LeakTracker::alive_count = 0;

    {
        SharedPtr<LeakTracker> sp_array(new LeakTracker[5], delete_leak_tracker_array);
        EXPECT_EQ(LeakTracker::alive_count, 5);
        EXPECT_EQ(sp_array.use_count(), 1);
    }

    EXPECT_EQ(LeakTracker::alive_count, 0);
}

TEST(SharedPtrDeleterTests, CustomFunctorDeleterTest) {
    LeakTracker::alive_count = 0;
    CustomFunctorDeleter::calls_count = 0;

    {
        CustomFunctorDeleter deleter;
        SharedPtr<LeakTracker> sp(new LeakTracker(), deleter);
        EXPECT_EQ(LeakTracker::alive_count, 1);
        EXPECT_EQ(CustomFunctorDeleter::calls_count, 0);
    }

    EXPECT_EQ(CustomFunctorDeleter::calls_count, 1);
    EXPECT_EQ(LeakTracker::alive_count, 0);
}

TEST(SharedPtrDeleterTests, ResetWithCustomDeleter) {
    LeakTracker::alive_count = 0;

    SharedPtr<LeakTracker> sp(new LeakTracker[2], delete_leak_tracker_array);
    EXPECT_EQ(LeakTracker::alive_count, 2);

    sp.reset();
    EXPECT_EQ(LeakTracker::alive_count, 0);
    EXPECT_EQ(sp.get(), nullptr);
    EXPECT_EQ(sp.use_count(), 0);

    sp.reset(new LeakTracker[5], delete_leak_tracker_array);
    EXPECT_EQ(LeakTracker::alive_count, 5);
    EXPECT_EQ(sp.use_count(), 1);

    sp.reset();
    EXPECT_EQ(LeakTracker::alive_count, 0);
    EXPECT_EQ(sp.use_count(), 0);
}

TEST(SharedPtrArrayTests, EmptyArrayPointer) {
    SharedPtr<LeakTracker[]> sp;
    EXPECT_EQ(sp.get(), nullptr);
    EXPECT_EQ(sp.use_count(), 0);
    EXPECT_FALSE(static_cast<bool>(sp));
}

TEST(SharedPtrArrayTests, SubscriptOperator) {
    SharedPtr<int[]> arr(new int[3]{10, 20, 30});

    EXPECT_NE(arr.get(), nullptr);
    EXPECT_EQ(arr.use_count(), 1);
    EXPECT_TRUE(static_cast<bool>(arr));

    EXPECT_EQ(arr[0], 10);
    EXPECT_EQ(arr[1], 20);
    EXPECT_EQ(arr[2], 30);

    arr[1] = 99;
    EXPECT_EQ(arr[1], 99);
}

TEST(SharedPtrArrayTests, AutomaticArrayDestruction) {
    LeakTracker::alive_count = 0;

    {
        SharedPtr<LeakTracker[]> arr(new LeakTracker[4]);
        EXPECT_EQ(LeakTracker::alive_count, 4);
        EXPECT_EQ(arr.use_count(), 1);
    }

    EXPECT_EQ(LeakTracker::alive_count, 0);
}

TEST(SharedPtrArrayTests, CopyConstructorAndAssignment) {
    LeakTracker::alive_count = 0;

    {
        SharedPtr<LeakTracker[]> sp1(new LeakTracker[3]);
        sp1[0].value = 100;
        EXPECT_EQ(sp1.use_count(), 1);
        EXPECT_EQ(LeakTracker::alive_count, 3);

        {
            SharedPtr<LeakTracker[]> sp2(sp1);
            EXPECT_EQ(sp1.get(), sp2.get());
            EXPECT_EQ(sp1.use_count(), 2);
            EXPECT_EQ(sp2.use_count(), 2);
            EXPECT_EQ(sp2[0].value, 100);

            sp2[0].value = 200;
            EXPECT_EQ(sp1[0].value, 200);
        }

        EXPECT_EQ(sp1.use_count(), 1);
        EXPECT_EQ(LeakTracker::alive_count, 3);

        SharedPtr<LeakTracker[]> sp3(new LeakTracker[2]);
        EXPECT_EQ(LeakTracker::alive_count, 5);

        sp3 = sp1;
        EXPECT_EQ(LeakTracker::alive_count, 3);
        EXPECT_EQ(sp1.use_count(), 2);
        EXPECT_EQ(sp3.use_count(), 2);
        EXPECT_EQ(sp3[0].value, 200);

        sp1 = sp1;
        EXPECT_EQ(sp1.use_count(), 2);
    }

    EXPECT_EQ(LeakTracker::alive_count, 0);
}

TEST(SharedPtrArrayTests, MoveConstructorAndAssignment) {
    LeakTracker::alive_count = 0;

    {
        SharedPtr<LeakTracker[]> sp1(new LeakTracker[3]);
        sp1[2].value = 55;
        EXPECT_EQ(LeakTracker::alive_count, 3);

        SharedPtr<LeakTracker[]> sp2(std::move(sp1));
        EXPECT_EQ(sp1.get(), nullptr);
        EXPECT_EQ(sp1.use_count(), 0);
        EXPECT_NE(sp2.get(), nullptr);
        EXPECT_EQ(sp2.use_count(), 1);
        EXPECT_EQ(sp2[2].value, 55);

        SharedPtr<LeakTracker[]> sp3(new LeakTracker[1]);
        EXPECT_EQ(LeakTracker::alive_count, 4);

        sp3 = std::move(sp2);
        EXPECT_EQ(sp2.get(), nullptr);
        EXPECT_EQ(sp2.use_count(), 0);
        EXPECT_NE(sp3.get(), nullptr);
        EXPECT_EQ(sp3.use_count(), 1);
        EXPECT_EQ(sp3[2].value, 55);
        EXPECT_EQ(LeakTracker::alive_count, 3);
    }

    EXPECT_EQ(LeakTracker::alive_count, 0);
}

TEST(SharedPtrArrayTests, ResetMethods) {
    LeakTracker::alive_count = 0;

    SharedPtr<LeakTracker[]> arr(new LeakTracker[3]);
    EXPECT_EQ(LeakTracker::alive_count, 3);
    EXPECT_EQ(arr.use_count(), 1);

    arr.reset();
    EXPECT_EQ(LeakTracker::alive_count, 0);
    EXPECT_EQ(arr.get(), nullptr);
    EXPECT_EQ(arr.use_count(), 0);

    arr.reset(new LeakTracker[5]);
    EXPECT_EQ(LeakTracker::alive_count, 5);
    EXPECT_EQ(arr.use_count(), 1);

    arr.reset(new LeakTracker[2]);
    EXPECT_EQ(LeakTracker::alive_count, 2);
    EXPECT_EQ(arr.use_count(), 1);

    arr.reset();
    EXPECT_EQ(LeakTracker::alive_count, 0);
    EXPECT_EQ(arr.use_count(), 0);
}