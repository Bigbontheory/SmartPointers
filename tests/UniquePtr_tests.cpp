
#include <gtest/gtest.h>
#include <utility>
#include <string>
#include <type_traits>
#include "UniquePtr.hpp"

struct BaseTracker {
    int value = 0;
    explicit BaseTracker(int v = 0) : value(v) {}
    virtual ~BaseTracker() = default;
};

struct DerivedTracker : public BaseTracker {
    explicit DerivedTracker(int v = 0) : BaseTracker(v) {}
    ~DerivedTracker() override = default;
};

struct StatefulDeleter {
    int calls_count = 0;

    void operator()(BaseTracker* ptr) {
        calls_count++;
        delete ptr;
    }
};

TEST(UniquePtrBase, DefaultAndNullptrConstructor) {
    UniquePtr<int> ptr_default;
    EXPECT_EQ(ptr_default.get(), nullptr);
    EXPECT_FALSE(static_cast<bool>(ptr_default));

    UniquePtr<int> ptr_explicit_nullptr(nullptr);
    EXPECT_EQ(ptr_explicit_nullptr.get(), nullptr);
    EXPECT_FALSE(static_cast<bool>(ptr_explicit_nullptr));
}

TEST(UniquePtrBase, RawPointerConstructorAndBoolOperator) {
    UniquePtr<BaseTracker> ptr(new BaseTracker(42));
    EXPECT_NE(ptr.get(), nullptr);
    EXPECT_TRUE(static_cast<bool>(ptr));
    EXPECT_EQ(ptr->value, 42);
}

TEST(UniquePtrBase, CopySemanticsAreDeleted) {
    EXPECT_FALSE((std::is_copy_constructible_v<UniquePtr<int>>));
    EXPECT_FALSE((std::is_copy_assignable_v<UniquePtr<int>>));
}

TEST(UniquePtrBase, MoveConstructorSameType) {
    UniquePtr<BaseTracker> first(new BaseTracker(10));
    BaseTracker* raw = first.get();

    UniquePtr<BaseTracker> second(std::move(first));

    EXPECT_EQ(first.get(), nullptr);
    EXPECT_EQ(second.get(), raw);
    EXPECT_EQ(second->value, 10);
}

TEST(UniquePtrBase, MoveAssignmentSameType) {
    UniquePtr<BaseTracker> first(new BaseTracker(10));
    UniquePtr<BaseTracker> second(new BaseTracker(20));
    BaseTracker* raw_first = first.get();

    second = std::move(first);

    EXPECT_EQ(first.get(), nullptr);
    EXPECT_EQ(second.get(), raw_first);
    EXPECT_EQ(second->value, 10);
}

TEST(UniquePtrBase, MoveAssignmentSelfAssignment) {
    UniquePtr<BaseTracker> ptr(new BaseTracker(99));
    BaseTracker* raw = ptr.get();

    ptr = std::move(ptr);

    EXPECT_EQ(ptr.get(), raw);
    EXPECT_EQ(ptr->value, 99);
}

TEST(UniquePtrPolymorphism, PolymorphicMoveConstructor) {
    UniquePtr<DerivedTracker> derived(new DerivedTracker(100));
    DerivedTracker* raw_derived = derived.get();

    UniquePtr<BaseTracker> base(std::move(derived));

    EXPECT_EQ(derived.get(), nullptr);
    EXPECT_EQ(base.get(), raw_derived);
    EXPECT_EQ(base->value, 100);
}

TEST(UniquePtrPolymorphism, PolymorphicMoveAssignment) {
    UniquePtr<BaseTracker> base(new BaseTracker(5));
    UniquePtr<DerivedTracker> derived(new DerivedTracker(50));
    DerivedTracker* raw_derived = derived.get();

    base = std::move(derived);

    EXPECT_EQ(derived.get(), nullptr);
    EXPECT_EQ(base.get(), raw_derived);
    EXPECT_EQ(base->value, 50);
}

TEST(UniquePtrBase, DereferenceOperators) {
    struct TestStruct {
        std::string text = "init";
    };

    UniquePtr<TestStruct> ptr(new TestStruct());

    EXPECT_EQ(ptr->text, "init");
    ptr->text = "modified";
    EXPECT_EQ(ptr->text, "modified");

    TestStruct& ref = *ptr;
    ref.text = "by_reference";
    EXPECT_EQ(ptr->text, "by_reference");
}

TEST(UniquePtrBase, ReleaseMethod) {
    UniquePtr<BaseTracker> ptr(new BaseTracker(7));

    BaseTracker* raw = ptr.release();

    EXPECT_EQ(ptr.get(), nullptr);
    EXPECT_FALSE(static_cast<bool>(ptr));
    EXPECT_NE(raw, nullptr);
    EXPECT_EQ(raw->value, 7);

    delete raw;
}

TEST(UniquePtrBase, ResetMethod) {
    UniquePtr<BaseTracker> ptr(new BaseTracker(1));

    ptr.reset(new BaseTracker(2));
    EXPECT_EQ(ptr->value, 2);

    ptr.reset();
    EXPECT_EQ(ptr.get(), nullptr);

    ptr.reset();
    EXPECT_EQ(ptr.get(), nullptr);
}

TEST(UniquePtrDeleter, StatefulDeleterAndGetDeleter) {
    StatefulDeleter d;
    {
        UniquePtr<BaseTracker, StatefulDeleter> ptr(new BaseTracker(15), d);
        EXPECT_EQ(ptr.get_deleter().calls_count, 0);

        const auto& const_ref = ptr;
        EXPECT_EQ(const_ref.get_deleter().calls_count, 0);

        ptr.get_deleter().calls_count = 10;
        EXPECT_EQ(ptr.get_deleter().calls_count, 10);
    }
}

TEST(UniquePtrDeleter, LambdaDeleterSupport) {
    bool lambda_called = false;
    auto my_deleter = [&lambda_called](int* p) {
        lambda_called = true;
        delete p;
    };

    {
        UniquePtr<int, decltype(my_deleter)> ptr(new int(100), my_deleter);
        EXPECT_FALSE(lambda_called);
    }
    EXPECT_TRUE(lambda_called);
}

TEST(UniquePtrArray, ConstructionAndDestruction) {
    UniquePtr<BaseTracker[]> arr(new BaseTracker[5]);
    EXPECT_NE(arr.get(), nullptr);
    EXPECT_TRUE(static_cast<bool>(arr));
}

TEST(UniquePtrArray, SubscriptOperator) {
    UniquePtr<int[]> arr(new int[3]{10, 20, 30});

    EXPECT_EQ(arr[0], 10);
    EXPECT_EQ(arr[1], 20);
    EXPECT_EQ(arr[2], 30);

    arr[1] = 999;
    EXPECT_EQ(arr[1], 999);
}

TEST(UniquePtrArray, MoveConstructor) {
    UniquePtr<BaseTracker[]> first(new BaseTracker[3]);
    BaseTracker* raw = first.get();

    UniquePtr<BaseTracker[]> second(std::move(first));

    EXPECT_EQ(first.get(), nullptr);
    EXPECT_EQ(second.get(), raw);
}

TEST(UniquePtrArray, MoveAssignmentAndSelfAssignment) {
    UniquePtr<BaseTracker[]> first(new BaseTracker[2]);
    UniquePtr<BaseTracker[]> second(new BaseTracker[4]);
    BaseTracker* raw_first = first.get();

    second = std::move(first);

    EXPECT_EQ(first.get(), nullptr);
    EXPECT_EQ(second.get(), raw_first);

    second = std::move(second);
    EXPECT_EQ(second.get(), raw_first);
}

TEST(UniquePtrArray, ReleaseAndReset) {
    UniquePtr<BaseTracker[]> arr(new BaseTracker[3]);

    BaseTracker* raw = arr.release();
    EXPECT_EQ(arr.get(), nullptr);
    EXPECT_NE(raw, nullptr);

    delete[] raw;

    arr.reset(new BaseTracker[2]);
    EXPECT_NE(arr.get(), nullptr);

    arr.reset();
    EXPECT_EQ(arr.get(), nullptr);
}

TEST(MakeUnique, SingleObjectForwarding) {
    struct MultiArgStruct {
        int a;
        std::string b;
        MultiArgStruct(int x, std::string y) : a(x), b(std::move(y)) {}
    };

    auto ptr = make_unique<MultiArgStruct>(42, "hello");
    EXPECT_NE(ptr.get(), nullptr);
    EXPECT_EQ(ptr->a, 42);
    EXPECT_EQ(ptr->b, "hello");
}

TEST(MakeUnique, DynamicArrayValueInitialization) {
    auto arr = make_unique<int[]>(10);
    EXPECT_NE(arr.get(), nullptr);

    for (std::size_t i = 0; i < 10; ++i) {
        EXPECT_EQ(arr[i], 0);
    }
}