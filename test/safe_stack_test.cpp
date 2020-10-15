#include "safe_stack/safe_stack.h"
#include "gtest/gtest.h"
#include <iostream>
#include <memory>

using namespace safe_stack;

TEST(Construction, Default) {
    Stack<int> s;
    EXPECT_TRUE(s.empty());
    EXPECT_THROW(s.pop(), StackUnderflow);
    EXPECT_THROW(s.top(), StackUnderflow);
    EXPECT_TRUE(s.valid());
}

TEST(Construction, CopyConstructor) {
    Stack<int> x;
    x.push(42);
    Stack<int> y{x};

    // check if the new stack also has 42.
    EXPECT_EQ(1, y.size());
    EXPECT_EQ(42, y.top());

    // old stack stays valid.
    x.pop();
    EXPECT_TRUE(x.empty());
}

TEST(Construction, CopyAssignment) {
    Stack<int> x;
    x.push(42);
    Stack<int> y;
    y.push(13);
    y = x;

    // check if the new stack has 42.
    EXPECT_EQ(1, y.size());
    EXPECT_EQ(42, y.top());

    // old stack stays valid.
    x.pop();
    EXPECT_TRUE(x.empty());
}

TEST(Construction, MoveConstructor) {
    Stack<int> x;
    x.push(42);
    Stack<int> y{std::move(x)};

    // check if the new stack also has 42.
    EXPECT_EQ(1, y.size());
    EXPECT_EQ(42, y.top());

    // check if the old stack is invalid.
    EXPECT_THROW(x.empty(), StackInvalidState);
}

TEST(Construction, MoveAssignment) {
    Stack<int> s;
    s.push(42);
    Stack<int> ss;
    ss.push(13);
    ss = std::move(s);

    // check if the new stack has 42.
    EXPECT_EQ(1, ss.size());
    EXPECT_EQ(42, ss.top());

    // check if the old stack is invalid.
    EXPECT_THROW(s.top(), StackInvalidState);
}

TEST(SafeStack, PopEmpty) {
    Stack<int> s;
    EXPECT_TRUE(s.empty());
    // EXPECT_TRUE(s.pop());
}

TEST(SafeStack, PushOneElement) {
    Stack<int> s;
    s.push(42);
    EXPECT_EQ(1, s.size());
    EXPECT_EQ(42, s.top());
}

TEST(Corruption, FillWithZeros) {
    Stack<int> s;
    EXPECT_TRUE(s.empty());
    std::uninitialized_fill_n(reinterpret_cast<char *>(&s), sizeof(s), 0);
    ASSERT_THROW(s.size(), StackInvalidState);
}

class ExecutionChamber : public testing::Test {
protected:
    int left[1];
    Stack<int> s;
    int right[1];
};

TEST_F(ExecutionChamber, FirstCanary) {
    EXPECT_TRUE(s.empty());
    *(left + 2) = 42;
    ASSERT_THROW(s.size(), StackInvalidState);
}

TEST_F(ExecutionChamber, SecondCanary) {
    EXPECT_TRUE(s.empty());
    *(right - 2) = 42;
    ASSERT_THROW(s.size(), StackInvalidState);
}
