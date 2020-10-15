#include "safe_stack/cache.h"
#include "gtest/gtest.h"

using namespace safe_stack;

TEST(Cache, Zeros) {
    struct Data {
        unsigned char a{0}, b{0}, c{0};
    };
    Data data;
    EXPECT_EQ(static_cast<CacheType>(1 * 31 * 31 * 31), cache(data));
}

TEST(Cache, LastNotNull) {
    struct Data {
        unsigned char values[4] = {0};
    };
    Data data;
    data.values[3] = 0xFF;
    EXPECT_EQ(static_cast<CacheType>(1 * 31 * 31 * 31 * 31 + data.values[3]), cache(data));
}
