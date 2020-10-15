#include <cstddef>
namespace safe_stack {

using CacheType = unsigned char;

constexpr CacheType cache_factor = 31;

template <class T>
CacheType cache(const T &data) {
    std::size_t size = sizeof(data);
    CacheType result = 1;
    for (std::size_t i = 0; i < size; ++i)
        result = cache_factor * result + reinterpret_cast<const CacheType *>(&data)[i];
    return result;
}
// 0 0 0 0 0 F 0 0
} // namespace safe_stack
