#include <cstddef>
namespace safe_stack {

/// \brief Data type of the hash (currently only one byte)
using HashType = unsigned char;

/// \brief Multiplier for every next operation of the hash algorithm
constexpr HashType hash_factor = 31;

/// \brief Computes a hash of the arbitrary value.
template <class T>
HashType hash(const T &data) {
    std::size_t size = sizeof(data);
    HashType result = 1;
    for (std::size_t i = 0; i < size; ++i)
        result =
            hash_factor * result + reinterpret_cast<const HashType *>(&data)[i];
    return result;
}

} // namespace safe_stack
