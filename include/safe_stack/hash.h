#include <cstddef>
namespace safe_stack {

using HashType = unsigned char;

constexpr HashType hash_factor = 31;

template <class T>
HashType hash(const T &data) {
    std::size_t size = sizeof(data);
    HashType result = 1;
    for (std::size_t i = 0; i < size; ++i)
        result = hash_factor * result + reinterpret_cast<const HashType *>(&data)[i];
    return result;
}
// 0 0 0 0 0 F 0 0
} // namespace safe_stack
