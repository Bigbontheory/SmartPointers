#include <gtest/gtest.h>
#include <utility>
#include "UniquePtr.hpp"
#include "mutable_array_sequence.hpp"

struct UniqueLeakTracker {
    static inline int alive_count = 0;

    UniqueLeakTracker() { alive_count++; }
    virtual ~UniqueLeakTracker() { alive_count--; }
};

struct DerivedUniqueLeakTracker : public UniqueLeakTracker {
    static inline int derived_alive_count = 0;

    DerivedUniqueLeakTracker() { derived_alive_count++; }
    ~DerivedUniqueLeakTracker() override { derived_alive_count--; }
};

TEST(UniquePtrTests, EmptyPointer) {
    UniquePtr<Sequence<int>> up;
    EXPECT_EQ(up.get(), nullptr);

    if (up) {
        FAIL();
    } else {
        SUCCEED();
    }
}

TEST(UniquePtr, ArrowDereferenceOperator) {
    UniquePtr<MutableArraySequence<int>> up(new MutableArraySequence<int>());

    up->append(30);
    up->append(20);
    up->append(50);

    EXPECT_EQ(up->get_size(), 3);
    EXPECT_EQ(up->get_first(), 30);
    EXPECT_EQ(up->get_last(), 50);

    if (up) {
        SUCCEED();
    } else {
        FAIL();
    }

    MutableArraySequence<int>& seq = *up;
    EXPECT_EQ(seq.get_size(), up->get_size());
    EXPECT_EQ(seq.get_first(), up->get_first());
    EXPECT_EQ(seq.get_last(), up->get_last());
}

TEST(UniquePtrTests, MoveTests) {
    UniquePtr<MutableArraySequence<int>> up1(new MutableArraySequence<int>());
    up1->append(42);
    MutableArraySequence<int>* raw_pointer = up1.get();

    UniquePtr<MutableArraySequence<int>> up2(std::move(up1));

    EXPECT_EQ(up1.get(), nullptr);
    EXPECT_EQ(up2.get(), raw_pointer);
    EXPECT_EQ(up2->get_first(), 42);

    UniquePtr<MutableArraySequence<int>> up3(new MutableArraySequence<int>());
    up3->append(49);

    up3 = std::move(up2);

    EXPECT_EQ(up3.get(), raw_pointer);
    EXPECT_EQ(up2.get(), nullptr);
    EXPECT_EQ(up3->get_first(), 42);

    up3 = std::move(up3);
    EXPECT_EQ(up3->get_first(), 42);
    EXPECT_EQ(up3.get(), raw_pointer);
}

TEST(UniquePtrLifetimeTests, ReleaseAndReset) {
    UniquePtr<MutableArraySequence<int>> uptr1(new MutableArraySequence<int>());
    uptr1->append(42);

    MutableArraySequence<int>* raw_ptr = uptr1.release();

    EXPECT_EQ(uptr1.get(), nullptr);
    EXPECT_NE(raw_ptr, nullptr);
    EXPECT_EQ(raw_ptr->get_first(), 42);

    delete raw_ptr;

    UniquePtr<MutableArraySequence<int>> uptr2(new MutableArraySequence<int>());
    uptr2->append(100);

    uptr2.reset();
    EXPECT_EQ(uptr2.get(), nullptr);

    UniquePtr<MutableArraySequence<int>> uptr3(new MutableArraySequence<int>());
    uptr3->append(1);

    auto replacement = new MutableArraySequence<int>();
    replacement->append(999);

    uptr3.reset(replacement);

    EXPECT_EQ(uptr3.get(), replacement);
    EXPECT_EQ(uptr3->get_first(), 999);
}

TEST(UniquePtrSubtypingTests, PolymorphicMoveUpcast) {
    UniquePtr<MutableArraySequence<int>> derived(new MutableArraySequence<int>());
    derived->append(777);

    MutableArraySequence<int>* raw_derived = derived.get();

    UniquePtr<Sequence<int>> base = std::move(derived);

    EXPECT_EQ(derived.get(), nullptr);
    EXPECT_EQ(base.get(), raw_derived);

    EXPECT_EQ(base->get_size(), 1);
    EXPECT_EQ(base->get_first(), 777);
}

TEST(UniquePtrMemoryLeaks, LifetimeAndScopeDestruction) {
    UniqueLeakTracker::alive_count = 0;

    {
        UniquePtr<UniqueLeakTracker> uptr(new UniqueLeakTracker());
        EXPECT_EQ(UniqueLeakTracker::alive_count, 1);
    }

    EXPECT_EQ(UniqueLeakTracker::alive_count, 0);
}

TEST(UniquePtrMemoryLeaks, ResetDestruction) {
    UniqueLeakTracker::alive_count = 0;

    UniquePtr<UniqueLeakTracker> uptr(new UniqueLeakTracker());
    EXPECT_EQ(UniqueLeakTracker::alive_count, 1);

    uptr.reset();

    EXPECT_EQ(UniqueLeakTracker::alive_count, 0);
    EXPECT_EQ(uptr.get(), nullptr);
}

TEST(UniquePtrMemoryLeaks, PolymorphicDestruction) {
    UniqueLeakTracker::alive_count = 0;
    DerivedUniqueLeakTracker::derived_alive_count = 0;

    {
        UniquePtr<DerivedUniqueLeakTracker> derived(new DerivedUniqueLeakTracker());
        EXPECT_EQ(UniqueLeakTracker::alive_count, 1);
        EXPECT_EQ(DerivedUniqueLeakTracker::derived_alive_count, 1);

        UniquePtr<UniqueLeakTracker> base = std::move(derived);
        EXPECT_EQ(UniqueLeakTracker::alive_count, 1);
        EXPECT_EQ(DerivedUniqueLeakTracker::derived_alive_count, 1);
    }

    EXPECT_EQ(DerivedUniqueLeakTracker::derived_alive_count, 0);
    EXPECT_EQ(UniqueLeakTracker::alive_count, 0);
}

void delete_unique_leak_tracker_array(void* p) {
    delete[] static_cast<UniqueLeakTracker*>(p);
}

void delete_derived_unique_leak_tracker_array(void* p) {
    delete[] static_cast<DerivedUniqueLeakTracker*>(p);
}

TEST(UniquePtrDeleterTests, ArrayDestructionCount) {
    UniqueLeakTracker::alive_count = 0;

    {
        UniquePtr<UniqueLeakTracker> uptr_array(new UniqueLeakTracker[5], delete_unique_leak_tracker_array);
        EXPECT_EQ(UniqueLeakTracker::alive_count, 5);
    }

    EXPECT_EQ(UniqueLeakTracker::alive_count, 0);
}

TEST(UniquePtrDeleterTests, SetDeleterViaMethod) {
    UniqueLeakTracker::alive_count = 0;

    {
        UniquePtr<UniqueLeakTracker> uptr(new UniqueLeakTracker[3]);
        EXPECT_EQ(UniqueLeakTracker::alive_count, 3);

        uptr.deleter(delete_unique_leak_tracker_array);
    }

    EXPECT_EQ(UniqueLeakTracker::alive_count, 0);
}

TEST(UniquePtrDeleterTests, MoveTransfersDeleter) {
    UniqueLeakTracker::alive_count = 0;

    {
        UniquePtr<UniqueLeakTracker> uptr1(new UniqueLeakTracker[4], delete_unique_leak_tracker_array);
        EXPECT_EQ(UniqueLeakTracker::alive_count, 4);

        UniquePtr<UniqueLeakTracker> uptr2 = std::move(uptr1);

        EXPECT_EQ(uptr1.get(), nullptr);
        EXPECT_NE(uptr2.get(), nullptr);
        EXPECT_EQ(UniqueLeakTracker::alive_count, 4);
    }

    EXPECT_EQ(UniqueLeakTracker::alive_count, 0);
}

TEST(UniquePtrDeleterTests, PolymorphicMoveTransfersDeleter) {
    UniqueLeakTracker::alive_count = 0;
    DerivedUniqueLeakTracker::derived_alive_count = 0;

    {
        UniquePtr<DerivedUniqueLeakTracker> derived(new DerivedUniqueLeakTracker[3], delete_derived_unique_leak_tracker_array);
        EXPECT_EQ(UniqueLeakTracker::alive_count, 3);
        EXPECT_EQ(DerivedUniqueLeakTracker::derived_alive_count, 3);

        UniquePtr<UniqueLeakTracker> base = std::move(derived);

        EXPECT_EQ(derived.get(), nullptr);
        EXPECT_NE(base.get(), nullptr);
    }

    EXPECT_EQ(DerivedUniqueLeakTracker::derived_alive_count, 0);
    EXPECT_EQ(UniqueLeakTracker::alive_count, 0);
}

TEST(UniquePtrDeleterTests, ResetWithCustomDeleter) {
    UniqueLeakTracker::alive_count = 0;

    UniquePtr<UniqueLeakTracker> uptr(new UniqueLeakTracker[2], delete_unique_leak_tracker_array);
    EXPECT_EQ(UniqueLeakTracker::alive_count, 2);

    uptr.reset();
    EXPECT_EQ(UniqueLeakTracker::alive_count, 0);
    EXPECT_EQ(uptr.get(), nullptr);
}

TEST(UniquePtrDeleterTests, ReleaseDoesNotInvokeDeleter) {
    UniqueLeakTracker::alive_count = 0;

    UniqueLeakTracker* raw_array = nullptr;

    {
        UniquePtr<UniqueLeakTracker> uptr(new UniqueLeakTracker[3], delete_unique_leak_tracker_array);
        EXPECT_EQ(UniqueLeakTracker::alive_count, 3);

        raw_array = uptr.release();
        EXPECT_EQ(uptr.get(), nullptr);
        EXPECT_EQ(UniqueLeakTracker::alive_count, 3);
    }

    EXPECT_EQ(UniqueLeakTracker::alive_count, 3);

    delete[] raw_array;
    EXPECT_EQ(UniqueLeakTracker::alive_count, 0);
}