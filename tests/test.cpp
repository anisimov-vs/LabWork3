// Anisimov Vasiliy st129629@student.spbu.ru
// Laboratory Work 3

#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <sstream>

#include "jump_list.h"

// Test fixture for basic jump_list tests
class JumpListTest : public ::testing::Test {
protected:
    void SetUp() override { }

    void TearDown() override { }

    jump_list<int> int_list;
    jump_list<std::string> str_list;
};

// Constructor Tests
TEST_F(JumpListTest, DefaultConstructor) {
    jump_list<int> jl;
    EXPECT_TRUE(jl.empty());
    EXPECT_EQ(jl.size(), 0);
    EXPECT_EQ(jl.begin(), jl.end());
}

TEST_F(JumpListTest, InitializerListConstructor) {
    jump_list<int> jl{5, 3, 8, 1, 7};
    EXPECT_EQ(jl.size(), 5);
    EXPECT_FALSE(jl.empty());
    
    // Check if elements are in sorted order
    std::vector<int> expected{1, 3, 5, 7, 8};
    std::vector<int> actual(jl.begin(), jl.end());
    EXPECT_EQ(actual, expected);
}

TEST_F(JumpListTest, RangeConstructor) {
    std::vector<int> vec{10, 20, 30, 40, 50};
    jump_list<int> jl(vec.begin(), vec.end());
    
    EXPECT_EQ(jl.size(), 5);
    std::vector<int> actual(jl.begin(), jl.end());
    EXPECT_EQ(actual, vec); // Already sorted
}

TEST_F(JumpListTest, CopyConstructor) {
    jump_list<int> original{5, 3, 8, 1, 7};
    jump_list<int> copy(original);
    
    EXPECT_EQ(copy.size(), original.size());
    EXPECT_TRUE(std::equal(copy.begin(), copy.end(), original.begin()));
}

TEST_F(JumpListTest, MoveConstructor) {
    jump_list<int> original{5, 3, 8, 1, 7};
    size_t original_size = original.size();
    
    // Create a copy of the original data for comparison
    std::vector<int> original_data(original.begin(), original.end());
    
    jump_list<int> moved(std::move(original));
    
    EXPECT_EQ(moved.size(), original_size);
    EXPECT_TRUE(original.empty()); // Original should be empty after move
    
    // Verify moved container has the correct data
    std::vector<int> moved_data(moved.begin(), moved.end());
    EXPECT_EQ(moved_data, original_data);
    
    // Verify original is still usable (shouldn't crash)
    original.insert(99);
    EXPECT_EQ(original.size(), 1);
    EXPECT_TRUE(original.contains(99));
}

// Assignment Tests
TEST_F(JumpListTest, CopyAssignment) {
    jump_list<int> original{5, 3, 8, 1, 7};
    jump_list<int> copy;
    copy = original;
    
    EXPECT_EQ(copy.size(), original.size());
    EXPECT_TRUE(std::equal(copy.begin(), copy.end(), original.begin()));
}

TEST_F(JumpListTest, MoveAssignment) {
    jump_list<int> original{5, 3, 8, 1, 7};
    size_t original_size = original.size();
    
    // Create a copy of the original data for comparison
    std::vector<int> original_data(original.begin(), original.end());
    
    jump_list<int> moved;
    moved = std::move(original);
    
    EXPECT_EQ(moved.size(), original_size);
    EXPECT_TRUE(original.empty());
    
    // Verify moved container has the correct data
    std::vector<int> moved_data(moved.begin(), moved.end());
    EXPECT_EQ(moved_data, original_data);
    
    // Verify original is still usable (shouldn't crash)
    original.insert(99);
    EXPECT_EQ(original.size(), 1);
    EXPECT_TRUE(original.contains(99));
}

TEST_F(JumpListTest, InitializerListAssignment) {
    jump_list<int> jl;
    jl = {10, 20, 30};
    
    EXPECT_EQ(jl.size(), 3);
    std::vector<int> expected{10, 20, 30};
    std::vector<int> actual(jl.begin(), jl.end());
    EXPECT_EQ(actual, expected);
}

// Capacity Tests
TEST_F(JumpListTest, EmptyAndSize) {
    EXPECT_TRUE(int_list.empty());
    EXPECT_EQ(int_list.size(), 0);
    
    int_list.insert(42);
    EXPECT_FALSE(int_list.empty());
    EXPECT_EQ(int_list.size(), 1);
}

TEST_F(JumpListTest, MaxSize) {
    EXPECT_GT(int_list.max_size(), 0);
}

// Iterator Tests
TEST_F(JumpListTest, IteratorTraversal) {
    std::vector<int> values{5, 3, 8, 1, 7, 4, 6, 2};
    for (int val : values) {
        int_list.insert(val);
    }
    
    // Check forward iteration
    std::vector<int> result;
    for (auto it = int_list.begin(); it != int_list.end(); ++it) {
        result.push_back(*it);
    }
    
    // Should be in sorted order
    std::sort(values.begin(), values.end());
    EXPECT_EQ(result, values);
}

TEST_F(JumpListTest, ConstIterator) {
    int_list.insert(1);
    int_list.insert(2);
    int_list.insert(3);
    
    const auto& const_list = int_list;
    std::vector<int> result;
    for (auto it = const_list.cbegin(); it != const_list.cend(); ++it) {
        result.push_back(*it);
    }
    
    std::vector<int> expected{1, 2, 3};
    EXPECT_EQ(result, expected);
}

TEST_F(JumpListTest, RangeBasedFor) {
    int_list.insert(10);
    int_list.insert(20);
    int_list.insert(30);
    
    std::vector<int> result;
    for (const auto& val : int_list) {
        result.push_back(val);
    }
    
    std::vector<int> expected{10, 20, 30};
    EXPECT_EQ(result, expected);
}

// Insert Tests
TEST_F(JumpListTest, InsertSingle) {
    auto result = int_list.insert(42);
    EXPECT_TRUE(result.second); // Should be inserted
    EXPECT_EQ(*result.first, 42);
    EXPECT_EQ(int_list.size(), 1);
}

TEST_F(JumpListTest, InsertDuplicate) {
    int_list.insert(42);
    auto result = int_list.insert(42);
    
    EXPECT_FALSE(result.second); // Should not be inserted
    EXPECT_EQ(*result.first, 42);
    EXPECT_EQ(int_list.size(), 1); // Size should remain 1
}

TEST_F(JumpListTest, InsertMove) {
    std::string str = "hello";
    auto result = str_list.insert(std::move(str));
    
    EXPECT_TRUE(result.second);
    EXPECT_EQ(*result.first, "hello");
    EXPECT_EQ(str_list.size(), 1);
}

TEST_F(JumpListTest, InsertRange) {
    std::vector<int> values{5, 3, 8, 1, 7};
    int_list.insert(values.begin(), values.end());
    
    EXPECT_EQ(int_list.size(), 5);
    
    std::vector<int> result(int_list.begin(), int_list.end());
    std::sort(values.begin(), values.end());
    EXPECT_EQ(result, values);
}

TEST_F(JumpListTest, InsertInitializerList) {
    int_list.insert({10, 20, 30, 40});
    
    EXPECT_EQ(int_list.size(), 4);
    std::vector<int> expected{10, 20, 30, 40};
    std::vector<int> actual(int_list.begin(), int_list.end());
    EXPECT_EQ(actual, expected);
}

// Emplace Tests
TEST_F(JumpListTest, Emplace) {
    auto result = str_list.emplace("test");
    EXPECT_TRUE(result.second);
    EXPECT_EQ(*result.first, "test");
    EXPECT_EQ(str_list.size(), 1);
}

TEST_F(JumpListTest, EmplaceHint) {
    auto it = str_list.emplace_hint(str_list.end(), "test");
    EXPECT_EQ(*it, "test");
    EXPECT_EQ(str_list.size(), 1);
}

// Erase Tests
TEST_F(JumpListTest, EraseByIterator) {
    int_list.insert({1, 2, 3, 4, 5});
    auto it = int_list.find(3);
    ASSERT_NE(it, int_list.end());
    
    auto next_it = int_list.erase(it);
    EXPECT_EQ(int_list.size(), 4);
    EXPECT_EQ(*next_it, 4);
    EXPECT_EQ(int_list.find(3), int_list.end());
}

TEST_F(JumpListTest, EraseByValue) {
    int_list.insert({1, 2, 3, 4, 5});
    size_t erased = int_list.erase(3);
    
    EXPECT_EQ(erased, 1);
    EXPECT_EQ(int_list.size(), 4);
    EXPECT_EQ(int_list.find(3), int_list.end());
}

TEST_F(JumpListTest, EraseNonExistent) {
    int_list.insert({1, 2, 3});
    size_t erased = int_list.erase(42);
    
    EXPECT_EQ(erased, 0);
    EXPECT_EQ(int_list.size(), 3);
}

TEST_F(JumpListTest, EraseRange) {
    int_list.insert({1, 2, 3, 4, 5});
    auto first = int_list.find(2);
    auto last = int_list.find(5); // Point to element 5, so we erase [2, 5) = {2, 3, 4}
    
    int_list.erase(first, last);
    EXPECT_EQ(int_list.size(), 2);
    
    std::vector<int> expected{1, 5};
    std::vector<int> actual(int_list.begin(), int_list.end());
    EXPECT_EQ(actual, expected);
}

TEST_F(JumpListTest, Clear) {
    int_list.insert({1, 2, 3, 4, 5});
    EXPECT_FALSE(int_list.empty());
    
    int_list.clear();
    EXPECT_TRUE(int_list.empty());
    EXPECT_EQ(int_list.size(), 0);
    EXPECT_EQ(int_list.begin(), int_list.end());
}

// Lookup Tests
TEST_F(JumpListTest, Find) {
    int_list.insert({1, 3, 5, 7, 9});
    
    auto it = int_list.find(5);
    EXPECT_NE(it, int_list.end());
    EXPECT_EQ(*it, 5);
    
    it = int_list.find(42);
    EXPECT_EQ(it, int_list.end());
}

TEST_F(JumpListTest, Count) {
    int_list.insert({1, 3, 5, 7, 9});
    
    EXPECT_EQ(int_list.count(5), 1);
    EXPECT_EQ(int_list.count(42), 0);
}

TEST_F(JumpListTest, Contains) {
    int_list.insert({1, 3, 5, 7, 9});
    
    EXPECT_TRUE(int_list.contains(5));
    EXPECT_FALSE(int_list.contains(42));
}

TEST_F(JumpListTest, LowerBound) {
    int_list.insert({1, 3, 5, 7, 9});
    
    auto it = int_list.lower_bound(5);
    EXPECT_EQ(*it, 5);
    
    it = int_list.lower_bound(4);
    EXPECT_EQ(*it, 5);
    
    it = int_list.lower_bound(10);
    EXPECT_EQ(it, int_list.end());
}

TEST_F(JumpListTest, UpperBound) {
    int_list.insert({1, 3, 5, 7, 9});
    
    auto it = int_list.upper_bound(5);
    EXPECT_EQ(*it, 7);
    
    it = int_list.upper_bound(4);
    EXPECT_EQ(*it, 5);
    
    it = int_list.upper_bound(9);
    EXPECT_EQ(it, int_list.end());
}

TEST_F(JumpListTest, EqualRange) {
    int_list.insert({1, 3, 5, 7, 9});
    
    auto range = int_list.equal_range(5);
    EXPECT_EQ(*range.first, 5);
    EXPECT_EQ(*range.second, 7);
    
    range = int_list.equal_range(42);
    EXPECT_EQ(range.first, range.second);
}

// Swap Test
TEST_F(JumpListTest, Swap) {
    jump_list<int> jl1{1, 2, 3};
    jump_list<int> jl2{4, 5, 6, 7};
    
    size_t size1 = jl1.size();
    size_t size2 = jl2.size();
    
    jl1.swap(jl2);
    
    EXPECT_EQ(jl1.size(), size2);
    EXPECT_EQ(jl2.size(), size1);
    
    std::vector<int> expected1{4, 5, 6, 7};
    std::vector<int> expected2{1, 2, 3};
    
    std::vector<int> actual1(jl1.begin(), jl1.end());
    std::vector<int> actual2(jl2.begin(), jl2.end());
    
    EXPECT_EQ(actual1, expected1);
    EXPECT_EQ(actual2, expected2);
}

// Observer Tests
TEST_F(JumpListTest, KeyComp) {
    auto comp = int_list.key_comp();
    EXPECT_TRUE(comp(1, 2));
    EXPECT_FALSE(comp(2, 1));
    EXPECT_FALSE(comp(1, 1));
}

TEST_F(JumpListTest, ValueComp) {
    auto comp = int_list.value_comp();
    EXPECT_TRUE(comp(1, 2));
    EXPECT_FALSE(comp(2, 1));
    EXPECT_FALSE(comp(1, 1));
}

// Comparison Operators Tests
TEST_F(JumpListTest, EqualityOperator) {
    jump_list<int> jl1{1, 2, 3};
    jump_list<int> jl2{1, 2, 3};
    jump_list<int> jl3{1, 2, 4};
    
    EXPECT_TRUE(jl1 == jl2);
    EXPECT_FALSE(jl1 == jl3);
    EXPECT_TRUE(jl1 != jl3);
    EXPECT_FALSE(jl1 != jl2);
}

TEST_F(JumpListTest, ComparisonOperators) {
    jump_list<int> jl1{1, 2, 3};
    jump_list<int> jl2{1, 2, 4};
    jump_list<int> jl3{1, 2, 3, 4};
    
    EXPECT_TRUE(jl1 < jl2);
    EXPECT_TRUE(jl1 < jl3);
    EXPECT_FALSE(jl2 < jl1);
    
    EXPECT_TRUE(jl1 <= jl2);
    EXPECT_TRUE(jl1 <= jl1);
    EXPECT_FALSE(jl2 <= jl1);
    
    EXPECT_TRUE(jl2 > jl1);
    EXPECT_FALSE(jl1 > jl2);
    
    EXPECT_TRUE(jl2 >= jl1);
    EXPECT_TRUE(jl1 >= jl1);
    EXPECT_FALSE(jl1 >= jl2);
}

// Performance and Stress Tests
TEST_F(JumpListTest, LargeDataset) {
    const int N = 10000;
    std::vector<int> values;
    values.reserve(N);
    
    // Generate random values
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100000);
    
    for (int i = 0; i < N; ++i) {
        values.push_back(dis(gen));
    }
    
    // Insert all values
    for (int val : values) {
        int_list.insert(val);
    }
    
    // Check that all unique values are present
    std::set<int> unique_values(values.begin(), values.end());
    EXPECT_EQ(int_list.size(), unique_values.size());
    
    // Verify sorted order
    std::vector<int> result(int_list.begin(), int_list.end());
    EXPECT_TRUE(std::is_sorted(result.begin(), result.end()));
}

TEST_F(JumpListTest, SequentialInsertAndSearch) {
    const int N = 1000;
    
    // Insert sequential values
    for (int i = 0; i < N; ++i) {
        int_list.insert(i);
    }
    
    EXPECT_EQ(int_list.size(), N);
    
    // Search for all values
    for (int i = 0; i < N; ++i) {
        EXPECT_TRUE(int_list.contains(i));
    }
    
    // Search for non-existent values
    for (int i = N; i < N + 100; ++i) {
        EXPECT_FALSE(int_list.contains(i));
    }
}

// Custom Comparator Test
TEST_F(JumpListTest, CustomComparator) {
    jump_list<int, std::greater<int>> desc_list;
    desc_list.insert({5, 3, 8, 1, 7});
    
    std::vector<int> result(desc_list.begin(), desc_list.end());
    std::vector<int> expected{8, 7, 5, 3, 1};
    
    EXPECT_EQ(result, expected);
}

// String Tests
TEST_F(JumpListTest, StringOperations) {
    str_list.insert({"banana", "apple", "cherry", "date"});
    
    EXPECT_EQ(str_list.size(), 4);
    
    std::vector<std::string> result(str_list.begin(), str_list.end());
    std::vector<std::string> expected{"apple", "banana", "cherry", "date"};
    
    EXPECT_EQ(result, expected);
    
    EXPECT_TRUE(str_list.contains("apple"));
    EXPECT_FALSE(str_list.contains("elderberry"));
}

// Edge Cases
TEST_F(JumpListTest, SingleElement) {
    int_list.insert(42);
    
    EXPECT_EQ(int_list.size(), 1);
    EXPECT_EQ(*int_list.begin(), 42);
    EXPECT_EQ(int_list.find(42), int_list.begin());
    
    int_list.erase(42);
    EXPECT_TRUE(int_list.empty());
}

TEST_F(JumpListTest, EraseFromSingleElement) {
    int_list.insert(42);
    auto it = int_list.find(42);
    auto next_it = int_list.erase(it);
    
    EXPECT_TRUE(int_list.empty());
    EXPECT_EQ(next_it, int_list.end());
}

TEST_F(JumpListTest, MultipleEraseSameValue) {
    int_list.insert(42);
    
    EXPECT_EQ(int_list.erase(42), 1);
    EXPECT_EQ(int_list.erase(42), 0); // Should not find it again
}

// Main function for running tests
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
