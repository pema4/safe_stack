#include "safe_stack/safe_stack.h"
#include "gtest/gtest.h"

using namespace safe_stack;

TEST(SafeStack, ConstructEmpty) {
    Stack<int> s;
    EXPECT_TRUE(s.empty());
    EXPECT_TRUE(s.pop());
    EXPECT_TRUE(s.top());
}

TEST(SafeStack, ConstructWithCopy) {
    Stack<int> x;
    (void)x.push(42);
    Stack<int> y{x};

    // chech if the new stack also has 42.
    EXPECT_EQ(1, y.size());
    EXPECT_EQ(42, y.top().result());

    // old stack stays valid.
    EXPECT_FALSE(x.pop());
    EXPECT_TRUE(x.empty());
}

TEST(SafeStack, CopyAssignment) {
    Stack<int> x;
    (void)x.push(42);
    Stack<int> y;
    (void)y.push(13);
    y = x;

    // check if the new stack has 42.
    EXPECT_EQ(1, y.size());
    EXPECT_EQ(42, y.top().result());

    // old stack stays valid.
    EXPECT_FALSE(x.pop());
    EXPECT_TRUE(x.empty());
}

TEST(SafeStack, ConstructWithMove) {
    Stack<int> x;
    (void)x.push(42);
    Stack<int> y{std::move(x)};

    // chech if the new stack also has 42.
    EXPECT_EQ(1, y.size());
    EXPECT_EQ(42, y.top().result());

    // check if the old stack is invalid.
    EXPECT_TRUE(x.pop());
}

TEST(SafeStack, MoveAssignment) {
    Stack<int> s;
    (void)s.push(42);
    Stack<int> ss;
    (void)ss.push(13);
    ss = std::move(s);

    // check if the new stack has 42.
    EXPECT_EQ(1, ss.size());
    EXPECT_EQ(42, ss.top().result());

    // check if the old stack is invalid.
    EXPECT_TRUE(s.top());
}

TEST(SafeStack, PopEmpty) {
    Stack<int> s;
    EXPECT_TRUE(s.empty());
    // EXPECT_TRUE(s.pop());
}

TEST(SafeStack, PushOneElement) {
    Stack<int> s;
    EXPECT_FALSE(s.push(42));
    EXPECT_EQ(1, s.size());
    auto res = s.top();
    EXPECT_FALSE(res);
    EXPECT_EQ(42, res.result());
}
