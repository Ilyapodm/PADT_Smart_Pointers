#include <gtest/gtest.h>

#include "unique_ptr.hpp"
#include "lifetime_counter.hpp"
#include <utility>

// anonymous namespace to prevent possible conflicts (there aren't any conflicts though)
namespace {

// Fixture before the test
class UniquePtrTest : public testing::Test {
protected:
    void SetUp() override {
        test_utils::LifetimeCounter::reset_counters();
    }
};

// use TEST_F for fixtures
TEST_F(UniquePtrTest, DefaultConstructedPointerIsEmpty) {
    UniquePtr<int> pointer;

    EXPECT_EQ(pointer.get(), nullptr);
    EXPECT_FALSE(pointer);
}

TEST_F(UniquePtrTest, ConstructedPointerOwnsObject) {
    UniquePtr<int> pointer(new int(42));

    ASSERT_TRUE(pointer);
    EXPECT_EQ(pointer.get(), &*pointer);
    EXPECT_EQ(*pointer, 42);
}

TEST_F(UniquePtrTest, ArrowOperatorProvidesObjectAccess) {
    struct Value {
        int number;
    };

    UniquePtr<Value> pointer(new Value{17});

    EXPECT_EQ(pointer->number, 17);
}

TEST_F(UniquePtrTest, OwnedObjectIsDestroyedWithPointer) {
    {
        UniquePtr<test_utils::LifetimeCounter> pointer(new test_utils::LifetimeCounter);

        EXPECT_EQ(test_utils::LifetimeCounter::created, 1);
        EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 0);
        EXPECT_EQ(test_utils::LifetimeCounter::alive, 1);
    }

    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 1);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 0);
}

TEST_F(UniquePtrTest, MoveTransfersOwnership) {
    UniquePtr<test_utils::LifetimeCounter> source(new test_utils::LifetimeCounter);
    test_utils::LifetimeCounter* raw_pointer = source.get();

    UniquePtr<test_utils::LifetimeCounter> destination(std::move(source));

    EXPECT_FALSE(source);
    EXPECT_EQ(destination.get(), raw_pointer);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 1);
}

TEST_F(UniquePtrTest, MoveAssignmentDestroysOldObject) {
    UniquePtr<test_utils::LifetimeCounter> source(new test_utils::LifetimeCounter);
    UniquePtr<test_utils::LifetimeCounter> destination(new test_utils::LifetimeCounter);
    test_utils::LifetimeCounter* source_pointer = source.get();

    destination = std::move(source);

    EXPECT_FALSE(source);
    EXPECT_EQ(destination.get(), source_pointer);
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 1);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 1);
}

TEST_F(UniquePtrTest, ReleaseReturnsObjectWithoutDestroyingIt) {
    UniquePtr<test_utils::LifetimeCounter> pointer(new test_utils::LifetimeCounter);

    test_utils::LifetimeCounter* released = pointer.release();

    EXPECT_FALSE(pointer);
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 0);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 1);

    // released ownership must be handled by the caller
    delete released;
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 1);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 0);
}

TEST_F(UniquePtrTest, ResetReplacesAndDestroysOwnedObject) {
    UniquePtr<test_utils::LifetimeCounter> pointer(new test_utils::LifetimeCounter);
    test_utils::LifetimeCounter* replacement = new test_utils::LifetimeCounter;

    pointer.reset(replacement);

    EXPECT_EQ(pointer.get(), replacement);
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 1);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 1);

    pointer.reset();
    EXPECT_FALSE(pointer);
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 2);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 0);
}

TEST_F(UniquePtrTest, ResetWithSamePointerKeepsObjectAlive) {
    UniquePtr<test_utils::LifetimeCounter> pointer(new test_utils::LifetimeCounter);
    test_utils::LifetimeCounter* raw_pointer = pointer.get();

    // resetting with the owned pointer must not delete it
    pointer.reset(raw_pointer);

    EXPECT_EQ(pointer.get(), raw_pointer);
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 0);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 1);
}

TEST_F(UniquePtrTest, SwapExchangesOwnedObjects) {
    UniquePtr<int> first(new int(10));
    UniquePtr<int> second(new int(20));
    int* first_pointer = first.get();
    int* second_pointer = second.get();

    first.swap(second);

    EXPECT_EQ(first.get(), second_pointer);
    EXPECT_EQ(second.get(), first_pointer);
    EXPECT_EQ(*first, 20);
    EXPECT_EQ(*second, 10);
}

TEST_F(UniquePtrTest, ArraySpecializationProvidesIndexedAccess) {
    UniquePtr<int[]> pointer(new int[3]{4, 5, 6});

    ASSERT_TRUE(pointer);
    EXPECT_EQ(pointer[0], 4);
    EXPECT_EQ(pointer[1], 5);
    EXPECT_EQ(pointer[2], 6);

    pointer[1] = 50;
    EXPECT_EQ(pointer[1], 50);
}

TEST_F(UniquePtrTest, ArraySpecializationDestroysEveryObject) {
    {
        UniquePtr<test_utils::LifetimeCounter[]> pointer(new test_utils::LifetimeCounter[3]);

        EXPECT_EQ(test_utils::LifetimeCounter::created, 3);
        EXPECT_EQ(test_utils::LifetimeCounter::alive, 3);
    }

    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 3);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 0);
}

TEST_F(UniquePtrTest, ArrayReleaseAndResetManageOwnership) {
    UniquePtr<test_utils::LifetimeCounter[]> pointer(new test_utils::LifetimeCounter[2]);

    test_utils::LifetimeCounter* released = pointer.release();
    EXPECT_FALSE(pointer);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 2);

    // arrays released from UniquePtr<T[]> require delete[]
    delete[] released;
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 2);

    pointer.reset(new test_utils::LifetimeCounter[3]);
    EXPECT_TRUE(pointer);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 3);

    pointer.reset();
    EXPECT_FALSE(pointer);
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 5);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 0);
}

}  // namespace
