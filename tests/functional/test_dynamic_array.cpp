#include <gtest/gtest.h>
#include <string>
#include <utility>

#include "dynamic_array.hpp"
#include "lifetime_counter.hpp"

namespace {

class DynamicArrayLifetimeTest : public testing::Test {
protected:
    void SetUp() override {
        test_utils::LifetimeCounter::reset_counters();
    }
};

TEST(DynamicArrayTest, DefaultAndZeroCountConstructorsAreEmpty) {
    DynamicArray<int> array;
    DynamicArray<int> zero_count(0);
    DynamicArray<int> null_source(nullptr, 0);

    EXPECT_EQ(array.get_size(), 0);
    EXPECT_EQ(array.get_capacity(), 0);
    EXPECT_EQ(zero_count.get_size(), 0);
    EXPECT_EQ(zero_count.get_capacity(), 0);
    EXPECT_EQ(null_source.get_size(), 0);
    EXPECT_EQ(null_source.get_capacity(), 0);
}

TEST(DynamicArrayTest, CountConstructorInitializesElements) {
    DynamicArray<int> array(3);

    EXPECT_EQ(array.get_size(), 3);
    EXPECT_EQ(array.get_capacity(), 3);
    EXPECT_EQ(array[0], 0);
    EXPECT_EQ(array[1], 0);
    EXPECT_EQ(array[2], 0);

    array[1] = 42;
    EXPECT_EQ(array.get(1), 42);
}

TEST(DynamicArrayTest, RawArrayConstructorCopiesValues) {
    int values[] = {4, 5, 6};
    DynamicArray<int> array(values, 3);
    const DynamicArray<int>& const_array = array;

    EXPECT_EQ(array.get_size(), 3);
    EXPECT_EQ(array.get_capacity(), 3);
    EXPECT_EQ(const_array[0], 4);
    EXPECT_EQ(const_array.get(1), 5);
    EXPECT_EQ(const_array[2], 6);

    values[0] = 10;
    array[1] = 20;
    EXPECT_EQ(array[0], 4);
    EXPECT_EQ(values[1], 5);
}

TEST(DynamicArrayTest, NullSourceWithNonzeroCountThrows) {
    EXPECT_THROW((DynamicArray<int>(nullptr, 1)), std::invalid_argument);
}

TEST(DynamicArrayTest, AccessOutsideSizeThrows) {
    DynamicArray<int> empty;
    DynamicArray<int> array;
    array.push_back(10);
    array.push_back(20);
    array.push_back(30);
    const DynamicArray<int>& const_array = array;

    EXPECT_THROW(empty.get(0), std::out_of_range);
    EXPECT_THROW(empty[0], std::out_of_range);
    // The last allocated slot is outside the logical size.
    EXPECT_THROW(array[3], std::out_of_range);
    EXPECT_THROW(const_array[3], std::out_of_range);
    EXPECT_THROW(const_array.get(3), std::out_of_range);
}

TEST(DynamicArrayTest, PushBackGrowsAndPreservesValues) {
    DynamicArray<int> array;
    array.push_back(10);
    EXPECT_EQ(array.get_size(), 1);
    EXPECT_EQ(array.get_capacity(), 1);

    array.push_back(20);
    EXPECT_EQ(array.get_capacity(), 2);
    array.push_back(30);
    EXPECT_EQ(array.get_capacity(), 4);
    int* first_element = &array[0];
    array.push_back(40);

    EXPECT_EQ(array.get_size(), 4);
    EXPECT_EQ(array.get_capacity(), 4);
    EXPECT_EQ(&array[0], first_element);
    for (std::size_t i = 0; i < array.get_size(); ++i) {
        EXPECT_EQ(array[i], (i + 1) * 10);
    }
}

TEST(DynamicArrayTest, PushBackOwnElementWorksDuringGrowth) {
    DynamicArray<std::string> array;
    array.push_back("value");
    array.push_back(array[0]);

    EXPECT_EQ(array.get_size(), 2);
    EXPECT_EQ(array[0], "value");
    EXPECT_EQ(array[1], "value");
}

TEST(DynamicArrayTest, CopyConstructorCreatesIndependentArray) {
    DynamicArray<int> source;
    source.push_back(10);
    source.push_back(20);
    source.push_back(30);
    DynamicArray<int> copy(source);

    EXPECT_EQ(copy.get_size(), 3);
    EXPECT_EQ(copy.get_capacity(), source.get_capacity());
    EXPECT_NE(&copy[0], &source[0]);
    EXPECT_EQ(copy[0], 10);
    EXPECT_EQ(copy[1], 20);
    EXPECT_EQ(copy[2], 30);
    copy[0] = 100;
    EXPECT_EQ(source[0], 10);
}

TEST(DynamicArrayTest, CopyAssignmentReplacesValuesWithIndependentCopy) {
    const int values[] = {10, 20};
    DynamicArray<int> source(values, 2);
    DynamicArray<int> destination(3);

    EXPECT_EQ(&(destination = source), &destination);
    EXPECT_EQ(destination.get_size(), 2);
    EXPECT_EQ(destination.get_capacity(), 2);
    EXPECT_NE(&destination[0], &source[0]);
    EXPECT_EQ(destination[0], 10);
    EXPECT_EQ(destination[1], 20);
    destination[1] = 100;
    EXPECT_EQ(source[1], 20);
}

TEST(DynamicArrayTest, MoveConstructorTransfersBufferAndSourceCanBeReused) {
    const int values[] = {10, 20};
    DynamicArray<int> source(values, 2);
    int* first_element = &source[0];
    DynamicArray<int> destination(std::move(source));

    EXPECT_EQ(&destination[0], first_element);
    EXPECT_EQ(destination.get_size(), 2);
    EXPECT_EQ(destination.get_capacity(), 2);
    EXPECT_EQ(destination[0], 10);
    EXPECT_EQ(destination[1], 20);
    EXPECT_EQ(source.get_size(), 0);
    EXPECT_EQ(source.get_capacity(), 0);

    source.push_back(30);
    EXPECT_EQ(source[0], 30);
    EXPECT_EQ(destination[0], 10);
}

TEST(DynamicArrayTest, MoveAssignmentTransfersBufferAndEmptiesSource) {
    const int values[] = {10, 20};
    DynamicArray<int> source(values, 2);
    DynamicArray<int> destination(3);
    int* first_element = &source[0];

    EXPECT_EQ(&(destination = std::move(source)), &destination);
    EXPECT_EQ(&destination[0], first_element);
    EXPECT_EQ(destination.get_size(), 2);
    EXPECT_EQ(destination.get_capacity(), 2);
    EXPECT_EQ(destination[0], 10);
    EXPECT_EQ(destination[1], 20);
    EXPECT_EQ(source.get_size(), 0);
    EXPECT_EQ(source.get_capacity(), 0);

    source.push_back(30);
    EXPECT_EQ(source[0], 30);
}

TEST(DynamicArrayTest, CopyAndMoveOfEmptyArraysStayEmpty) {
    DynamicArray<int> empty;
    DynamicArray<int> copy(empty);
    DynamicArray<int> moved(std::move(empty));

    EXPECT_EQ(copy.get_size(), 0);
    EXPECT_EQ(copy.get_capacity(), 0);
    EXPECT_EQ(moved.get_size(), 0);
    EXPECT_EQ(moved.get_capacity(), 0);
    EXPECT_EQ(empty.get_size(), 0);
    EXPECT_EQ(empty.get_capacity(), 0);
}

TEST(DynamicArrayTest, SelfAssignmentPreservesBufferAndValues) {
    const int values[] = {10, 20};
    DynamicArray<int> array(values, 2);
    DynamicArray<int>& same_array = array;
    int* first_element = &array[0];

    EXPECT_EQ(&(array = same_array), &array);
    EXPECT_EQ(&(array = std::move(same_array)), &array);
    EXPECT_EQ(&array[0], first_element);
    EXPECT_EQ(array.get_size(), 2);
    EXPECT_EQ(array.get_capacity(), 2);
    EXPECT_EQ(array[0], 10);
    EXPECT_EQ(array[1], 20);
}

TEST_F(DynamicArrayLifetimeTest, DestructorDestroysEveryAllocatedElement) {
    {
        DynamicArray<test_utils::LifetimeCounter> array(3);
        EXPECT_EQ(test_utils::LifetimeCounter::alive, 3);
    }

    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 3);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 0);
}

TEST_F(DynamicArrayLifetimeTest, GrowthReleasesOldBuffer) {
    {
        test_utils::LifetimeCounter value;
        DynamicArray<test_utils::LifetimeCounter> array;
        array.push_back(value);
        array.push_back(value);
        array.push_back(value);

        EXPECT_EQ(array.get_size(), 3);
        EXPECT_EQ(array.get_capacity(), 4);
        EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 3);
        EXPECT_EQ(test_utils::LifetimeCounter::alive, 5);
    }

    EXPECT_EQ(test_utils::LifetimeCounter::alive, 0);
    EXPECT_EQ(test_utils::LifetimeCounter::created, test_utils::LifetimeCounter::destroyed);
}

TEST_F(DynamicArrayLifetimeTest, AssignmentsReleaseOldBuffers) {
    {
        DynamicArray<test_utils::LifetimeCounter> source(2);
        DynamicArray<test_utils::LifetimeCounter> destination(3);
        destination = source;
        EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 3);
        EXPECT_EQ(test_utils::LifetimeCounter::alive, 4);

        destination = std::move(source);
        EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 5);
        EXPECT_EQ(test_utils::LifetimeCounter::alive, 2);
    }

    EXPECT_EQ(test_utils::LifetimeCounter::alive, 0);
    EXPECT_EQ(test_utils::LifetimeCounter::created, test_utils::LifetimeCounter::destroyed);
}

TEST_F(DynamicArrayLifetimeTest, AssignmentFromEmptyReleasesBuffers) {
    DynamicArray<test_utils::LifetimeCounter> empty;
    DynamicArray<test_utils::LifetimeCounter> first(2);
    DynamicArray<test_utils::LifetimeCounter> second(3);

    first = empty;
    second = std::move(empty);

    EXPECT_EQ(first.get_size(), 0);
    EXPECT_EQ(first.get_capacity(), 0);
    EXPECT_EQ(second.get_size(), 0);
    EXPECT_EQ(second.get_capacity(), 0);
    EXPECT_EQ(test_utils::LifetimeCounter::destroyed, 5);
    EXPECT_EQ(test_utils::LifetimeCounter::alive, 0);
}

}  // namespace
