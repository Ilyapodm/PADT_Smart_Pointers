#include <gtest/gtest.h>
#include <utility>

#include "shared_ptr.hpp"
#include "lifetime_counter.hpp"

namespace {

class SharedPtrLifetimeTest : public testing::Test {
protected:
    void SetUp() override {
        test_utils::LifetimeCounter::reset_counters();
    }
};

TEST(SharedPtrTest, DereferenceAndArrowProvideObjectAccess) {
    SharedPtr<std::pair<int, int>> pointer(new std::pair<int, int>{17, 42});

    ASSERT_TRUE(pointer);
    EXPECT_EQ(pointer.get(), &*pointer);
    EXPECT_EQ(pointer->first, 17);
    pointer->second = 50;
    EXPECT_EQ((*pointer).second, 50);
}

TEST(SharedPtrTest, CopySharesChangesToObject) {
    SharedPtr<int> pointer(new int(42));
    SharedPtr<int> copy(pointer);

    *copy = 17;

    EXPECT_EQ(*pointer, 17);
}

TEST(SharedPtrArrayTest, CopySharesChangesToArray) {
    SharedPtr<int[]> pointer(new int[3]{4, 5, 6});
    SharedPtr<int[]> copy(pointer);

    copy[1] = 50;

    EXPECT_EQ(pointer[1], 50);
}

TEST(SharedPtrTest, ConstPointerProvidesMutableObjectAccess) {
    const SharedPtr<int> pointer(new int(42));

    ASSERT_TRUE(pointer);
    EXPECT_EQ(*pointer, 42);
    *pointer = 17;
    EXPECT_EQ(*pointer, 17);
}

TEST(SharedPtrArrayTest, ConstPointerProvidesIndexedAccess) {
    const SharedPtr<int[]> pointer(new int[3]{4, 5, 6});

    ASSERT_TRUE(pointer);
    EXPECT_EQ(pointer[0], 4);
    EXPECT_EQ(pointer[1], 5);
    EXPECT_EQ(pointer[2], 6);
    EXPECT_EQ(&pointer[0], pointer.get());

    pointer[1] = 50;
    EXPECT_EQ(pointer[1], 50);
}

TEST(SharedPtrTest, DefaultAndNullPointersAreEmpty) {
    SharedPtr<int> pointer;
    SharedPtr<int> null_pointer(nullptr);

    EXPECT_EQ(pointer.get(), nullptr);
    EXPECT_FALSE(pointer);
    EXPECT_EQ(null_pointer.get(), nullptr);
    EXPECT_FALSE(null_pointer);
}

TEST(SharedPtrTest, CopyAndMoveOfEmptyPointerStayEmpty) {
    const SharedPtr<int> empty;
    SharedPtr<int> copy(empty);
    SharedPtr<int> moved(std::move(copy));

    EXPECT_EQ(copy.get(), nullptr);
    EXPECT_FALSE(copy);
    EXPECT_EQ(moved.get(), nullptr);
    EXPECT_FALSE(moved);
}

TEST_F(SharedPtrLifetimeTest, LastOwnerDestroysOwnedObjects) {
    {
        SharedPtr<test_utils::LifetimeCounter> pointer(new test_utils::LifetimeCounter);
        {
            const SharedPtr<test_utils::LifetimeCounter> copy(pointer);
            SharedPtr<test_utils::LifetimeCounter> another_copy(copy);

            EXPECT_EQ(copy.get(), pointer.get());
            EXPECT_EQ(another_copy.get(), pointer.get());
            EXPECT_EQ(test_utils::LifetimeCounter::created, 1);
            EXPECT_EQ(test_utils::LifetimeCounter::alive, 1);
        }  // copies namespace

        EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 0);
        EXPECT_EQ(test_utils::LifetimeCounter::alive, 1);
    } // pointer namespace

    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 1);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 0);
}

TEST_F(SharedPtrLifetimeTest, CopyAssignmentReplacesOldOwnership) {
    SharedPtr<test_utils::LifetimeCounter> source(new test_utils::LifetimeCounter);
    SharedPtr<test_utils::LifetimeCounter> destination(new test_utils::LifetimeCounter);

    EXPECT_EQ(&(destination = source), &destination);
    EXPECT_EQ(destination.get(), source.get());
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 1);

    source.reset();
    EXPECT_TRUE(destination);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 1);

    destination.reset();
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 2);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 0);
}

TEST_F(SharedPtrLifetimeTest, MoveTransfersSharedOwnership) {
    SharedPtr<test_utils::LifetimeCounter> source(new test_utils::LifetimeCounter);
    SharedPtr<test_utils::LifetimeCounter> copy(source);
    test_utils::LifetimeCounter* raw_pointer = source.get();

    SharedPtr<test_utils::LifetimeCounter> destination(std::move(source));

    EXPECT_EQ(source.get(), nullptr);
    EXPECT_FALSE(source);
    EXPECT_EQ(destination.get(), raw_pointer);
    EXPECT_EQ(copy.get(), raw_pointer);

    copy.reset();
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 0);
    destination.reset();
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 1);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 0);
}

TEST_F(SharedPtrLifetimeTest, MoveAssignmentDestroysOldObjects) {
    SharedPtr<test_utils::LifetimeCounter> source(new test_utils::LifetimeCounter);
    SharedPtr<test_utils::LifetimeCounter> destination(new test_utils::LifetimeCounter);
    test_utils::LifetimeCounter* raw_pointer = source.get();

    EXPECT_EQ(&(destination = std::move(source)), &destination);
    EXPECT_EQ(source.get(), nullptr);
    EXPECT_FALSE(source);
    EXPECT_EQ(destination.get(), raw_pointer);
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 1);

    destination.reset();
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 2);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 0);
}

TEST_F(SharedPtrLifetimeTest, AssignmentFromEmptyReleasesOwnership) {
    SharedPtr<test_utils::LifetimeCounter> empty;
    SharedPtr<test_utils::LifetimeCounter> first(new test_utils::LifetimeCounter);
    SharedPtr<test_utils::LifetimeCounter> second(new test_utils::LifetimeCounter);

    first = empty;
    second = std::move(empty);

    EXPECT_EQ(first.get(), nullptr);
    EXPECT_FALSE(first);
    EXPECT_EQ(second.get(), nullptr);
    EXPECT_FALSE(second);
    EXPECT_FALSE(empty);
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 2);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 0);
}

TEST_F(SharedPtrLifetimeTest, SelfAssignmentKeepsOwnership) {
    SharedPtr<test_utils::LifetimeCounter> pointer(new test_utils::LifetimeCounter);
    SharedPtr<test_utils::LifetimeCounter>& same_pointer = pointer;
    test_utils::LifetimeCounter* raw_pointer = pointer.get();

    EXPECT_EQ(&(pointer = same_pointer), &pointer);
    EXPECT_EQ(&(pointer = std::move(same_pointer)), &pointer);
    EXPECT_EQ(pointer.get(), raw_pointer);
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 0);

    pointer.reset();
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 1);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 0);
}

TEST_F(SharedPtrLifetimeTest, AssignmentBetweenOwnersOfSameObjects) {
    SharedPtr<test_utils::LifetimeCounter> first(new test_utils::LifetimeCounter);
    SharedPtr<test_utils::LifetimeCounter> second(first);
    test_utils::LifetimeCounter* raw_pointer = first.get();

    first = second;
    EXPECT_EQ(first.get(), raw_pointer);
    EXPECT_EQ(second.get(), raw_pointer);
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 0);

    first = std::move(second);
    EXPECT_EQ(first.get(), raw_pointer);
    EXPECT_EQ(second.get(), nullptr);
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 0);

    first.reset();
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 1);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 0);
}

TEST_F(SharedPtrLifetimeTest, ResetReplacesOwnershipAndPreservesOtherOwners) {
    SharedPtr<test_utils::LifetimeCounter> pointer(new test_utils::LifetimeCounter);
    SharedPtr<test_utils::LifetimeCounter> copy(pointer);
    test_utils::LifetimeCounter* original = copy.get();
    test_utils::LifetimeCounter* replacement = new test_utils::LifetimeCounter;

    pointer.reset(replacement);

    EXPECT_EQ(pointer.get(), replacement);
    EXPECT_EQ(copy.get(), original);
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 0);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 2);

    copy.reset();
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 1);
    pointer.reset();
    EXPECT_EQ(pointer.get(), nullptr);
    EXPECT_FALSE(pointer);
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 2);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 0);
}

TEST_F(SharedPtrLifetimeTest, ResetReplacesLastOwnedObjectsAndReusesEmptyPointer) {
    SharedPtr<test_utils::LifetimeCounter> pointer(new test_utils::LifetimeCounter);
    test_utils::LifetimeCounter* replacement = new test_utils::LifetimeCounter;

    pointer.reset(replacement);
    EXPECT_EQ(pointer.get(), replacement);
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 1);

    pointer.reset();
    pointer.reset(nullptr);
    EXPECT_EQ(pointer.get(), nullptr);
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 2);

    pointer.reset(new test_utils::LifetimeCounter);
    EXPECT_TRUE(pointer);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 1);
    pointer.reset();
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 3);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 0);
}

TEST_F(SharedPtrLifetimeTest, SwapExchangesPointersAndOwnershipGroups) {
    SharedPtr<test_utils::LifetimeCounter> first(new test_utils::LifetimeCounter);
    SharedPtr<test_utils::LifetimeCounter> copy(first);
    SharedPtr<test_utils::LifetimeCounter> second(new test_utils::LifetimeCounter);
    test_utils::LifetimeCounter* first_pointer = first.get();
    test_utils::LifetimeCounter* second_pointer = second.get();

    first.swap(second);

    EXPECT_EQ(first.get(), second_pointer);
    EXPECT_EQ(second.get(), first_pointer);
    EXPECT_EQ(copy.get(), first_pointer);

    first.reset();
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 1);
    second.reset();
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 1);
    copy.reset();
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 2);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 0);
}

TEST_F(SharedPtrLifetimeTest, SwapWithEmptyAndSelfKeepsObjectsAlive) {
    SharedPtr<test_utils::LifetimeCounter> pointer(new test_utils::LifetimeCounter);
    SharedPtr<test_utils::LifetimeCounter> empty;
    test_utils::LifetimeCounter* raw_pointer = pointer.get();

    pointer.swap(pointer);
    EXPECT_EQ(pointer.get(), raw_pointer);
    pointer.swap(empty);
    EXPECT_EQ(pointer.get(), nullptr);
    EXPECT_FALSE(pointer);
    EXPECT_EQ(empty.get(), raw_pointer);
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 0);

    empty.reset();
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 1);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 0);
}

TEST(SharedPtrArrayTest, DefaultAndNullPointersAreEmpty) {
    SharedPtr<int[]> pointer;
    SharedPtr<int[]> null_pointer(nullptr);

    EXPECT_EQ(pointer.get(), nullptr);
    EXPECT_FALSE(pointer);
    EXPECT_EQ(null_pointer.get(), nullptr);
    EXPECT_FALSE(null_pointer);
}

TEST(SharedPtrArrayTest, CopyAndMoveOfEmptyPointerStayEmpty) {
    const SharedPtr<int[]> empty;
    SharedPtr<int[]> copy(empty);
    SharedPtr<int[]> moved(std::move(copy));

    EXPECT_EQ(copy.get(), nullptr);
    EXPECT_FALSE(copy);
    EXPECT_EQ(moved.get(), nullptr);
    EXPECT_FALSE(moved);
}

TEST_F(SharedPtrLifetimeTest, ArrayLastOwnerDestroysOwnedObjects) {
    {
        SharedPtr<test_utils::LifetimeCounter[]> pointer(new test_utils::LifetimeCounter[3]);
        {
            const SharedPtr<test_utils::LifetimeCounter[]> copy(pointer);
            SharedPtr<test_utils::LifetimeCounter[]> another_copy(copy);

            EXPECT_EQ(copy.get(), pointer.get());
            EXPECT_EQ(another_copy.get(), pointer.get());
            EXPECT_EQ(test_utils::LifetimeCounter::created, 3);
            EXPECT_EQ(test_utils::LifetimeCounter::alive, 3);
        }

        EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 0);
        EXPECT_EQ(test_utils::LifetimeCounter::alive, 3);
    }

    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 3);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 0);
}

TEST_F(SharedPtrLifetimeTest, ArrayCopyAssignmentReplacesOldOwnership) {
    SharedPtr<test_utils::LifetimeCounter[]> source(new test_utils::LifetimeCounter[3]);
    SharedPtr<test_utils::LifetimeCounter[]> destination(new test_utils::LifetimeCounter[3]);

    EXPECT_EQ(&(destination = source), &destination);
    EXPECT_EQ(destination.get(), source.get());
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 3);

    source.reset();
    EXPECT_TRUE(destination);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 3);

    destination.reset();
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 6);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 0);
}

TEST_F(SharedPtrLifetimeTest, ArrayMoveTransfersSharedOwnership) {
    SharedPtr<test_utils::LifetimeCounter[]> source(new test_utils::LifetimeCounter[3]);
    SharedPtr<test_utils::LifetimeCounter[]> copy(source);
    test_utils::LifetimeCounter* raw_pointer = source.get();

    SharedPtr<test_utils::LifetimeCounter[]> destination(std::move(source));

    EXPECT_EQ(source.get(), nullptr);
    EXPECT_FALSE(source);
    EXPECT_EQ(destination.get(), raw_pointer);
    EXPECT_EQ(copy.get(), raw_pointer);

    copy.reset();
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 0);
    destination.reset();
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 3);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 0);
}

TEST_F(SharedPtrLifetimeTest, ArrayMoveAssignmentDestroysOldObjects) {
    SharedPtr<test_utils::LifetimeCounter[]> source(new test_utils::LifetimeCounter[3]);
    SharedPtr<test_utils::LifetimeCounter[]> destination(new test_utils::LifetimeCounter[3]);
    test_utils::LifetimeCounter* raw_pointer = source.get();

    EXPECT_EQ(&(destination = std::move(source)), &destination);
    EXPECT_EQ(source.get(), nullptr);
    EXPECT_FALSE(source);
    EXPECT_EQ(destination.get(), raw_pointer);
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 3);

    destination.reset();
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 6);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 0);
}

TEST_F(SharedPtrLifetimeTest, ArrayAssignmentFromEmptyReleasesOwnership) {
    SharedPtr<test_utils::LifetimeCounter[]> empty;
    SharedPtr<test_utils::LifetimeCounter[]> first(new test_utils::LifetimeCounter[3]);
    SharedPtr<test_utils::LifetimeCounter[]> second(new test_utils::LifetimeCounter[3]);

    first = empty;
    second = std::move(empty);

    EXPECT_EQ(first.get(), nullptr);
    EXPECT_FALSE(first);
    EXPECT_EQ(second.get(), nullptr);
    EXPECT_FALSE(second);
    EXPECT_FALSE(empty);
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 6);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 0);
}

TEST_F(SharedPtrLifetimeTest, ArraySelfAssignmentKeepsOwnership) {
    SharedPtr<test_utils::LifetimeCounter[]> pointer(new test_utils::LifetimeCounter[3]);
    SharedPtr<test_utils::LifetimeCounter[]>& same_pointer = pointer;
    test_utils::LifetimeCounter* raw_pointer = pointer.get();

    EXPECT_EQ(&(pointer = same_pointer), &pointer);
    EXPECT_EQ(&(pointer = std::move(same_pointer)), &pointer);
    EXPECT_EQ(pointer.get(), raw_pointer);
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 0);

    pointer.reset();
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 3);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 0);
}

TEST_F(SharedPtrLifetimeTest, ArrayAssignmentBetweenOwnersOfSameObjects) {
    SharedPtr<test_utils::LifetimeCounter[]> first(new test_utils::LifetimeCounter[3]);
    SharedPtr<test_utils::LifetimeCounter[]> second(first);
    test_utils::LifetimeCounter* raw_pointer = first.get();

    first = second;
    EXPECT_EQ(first.get(), raw_pointer);
    EXPECT_EQ(second.get(), raw_pointer);
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 0);

    first = std::move(second);
    EXPECT_EQ(first.get(), raw_pointer);
    EXPECT_EQ(second.get(), nullptr);
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 0);

    first.reset();
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 3);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 0);
}

TEST_F(SharedPtrLifetimeTest, ArrayResetReplacesOwnershipAndPreservesOtherOwners) {
    SharedPtr<test_utils::LifetimeCounter[]> pointer(new test_utils::LifetimeCounter[3]);
    SharedPtr<test_utils::LifetimeCounter[]> copy(pointer);
    test_utils::LifetimeCounter* original = copy.get();
    test_utils::LifetimeCounter* replacement = new test_utils::LifetimeCounter[3];

    pointer.reset(replacement);

    EXPECT_EQ(pointer.get(), replacement);
    EXPECT_EQ(copy.get(), original);
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 0);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 6);

    copy.reset();
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 3);
    pointer.reset();
    EXPECT_EQ(pointer.get(), nullptr);
    EXPECT_FALSE(pointer);
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 6);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 0);
}

TEST_F(SharedPtrLifetimeTest, ArrayResetReplacesLastOwnedObjectsAndReusesEmptyPointer) {
    SharedPtr<test_utils::LifetimeCounter[]> pointer(new test_utils::LifetimeCounter[3]);
    test_utils::LifetimeCounter* replacement = new test_utils::LifetimeCounter[3];

    pointer.reset(replacement);
    EXPECT_EQ(pointer.get(), replacement);
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 3);

    pointer.reset();
    pointer.reset(nullptr);
    EXPECT_EQ(pointer.get(), nullptr);
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 6);

    pointer.reset(new test_utils::LifetimeCounter[3]);
    EXPECT_TRUE(pointer);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 3);
    pointer.reset();
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 9);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 0);
}

TEST_F(SharedPtrLifetimeTest, ArraySwapExchangesPointersAndOwnershipGroups) {
    SharedPtr<test_utils::LifetimeCounter[]> first(new test_utils::LifetimeCounter[3]);
    SharedPtr<test_utils::LifetimeCounter[]> copy(first);
    SharedPtr<test_utils::LifetimeCounter[]> second(new test_utils::LifetimeCounter[3]);
    test_utils::LifetimeCounter* first_pointer = first.get();
    test_utils::LifetimeCounter* second_pointer = second.get();

    first.swap(second);

    EXPECT_EQ(first.get(), second_pointer);
    EXPECT_EQ(second.get(), first_pointer);
    EXPECT_EQ(copy.get(), first_pointer);

    first.reset();
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 3);
    second.reset();
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 3);
    copy.reset();
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 6);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 0);
}

TEST_F(SharedPtrLifetimeTest, ArraySwapWithEmptyAndSelfKeepsObjectsAlive) {
    SharedPtr<test_utils::LifetimeCounter[]> pointer(new test_utils::LifetimeCounter[3]);
    SharedPtr<test_utils::LifetimeCounter[]> empty;
    test_utils::LifetimeCounter* raw_pointer = pointer.get();

    pointer.swap(pointer);
    EXPECT_EQ(pointer.get(), raw_pointer);
    pointer.swap(empty);
    EXPECT_EQ(pointer.get(), nullptr);
    EXPECT_FALSE(pointer);
    EXPECT_EQ(empty.get(), raw_pointer);
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 0);

    empty.reset();
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 3);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 0);
}

}  // namespace
