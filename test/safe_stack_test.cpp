#include "safe_stack/safe_stack.h"
#include "gtest/gtest.h"
#include <string>

TEST(hello_world, should_pass) {
    std::string a = "hello";
    std::string b = "world";
    ASSERT_NE(a, b);
}
