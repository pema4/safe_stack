#ifndef SAFE_STACK_H
#define SAFE_STACK_H

#include <cassert> // for assert
#include <cstdint> // for std::uintptr_t
#include <memory>

namespace safe_stack {

struct StackError {};

struct StackUnderflow : public StackError {};

struct StackInvalidState : public StackError {};

/// \brief Safe stack class.
/// Main design decisions (maybe they are wrong, I am not sure):
/// 1.
/// 2.
/// 3. If object is moved out to a new place, it is marked as invalid
/// (`size` becomes bigger than `capacity). Any operation on invalid object
/// returns ::StackError::INVALID_STATE
///
template <class T, class Allocator = std::allocator<T>>
class Stack {
public:
    /// \brief Constructs an empty stack.
    /// This function never fails, becaus
    Stack() = default;

    /// \brief Constructs a copy of the stack.
    Stack(const Stack &o)
        : _capacity{o._size}, _size{o._size}, _allocator{o._allocator} {
        set_data(allocator_traits::allocate(_allocator, _capacity));
        std::uninitialized_copy_n(o._data, _size, _data);
    }

    Stack &operator=(const Stack &o) {
        if (this == &o)
            return *this;

        set_capacity(o._capacity);
        set_size(o._size);
        _allocator = o._allocator;
        set_data(allocator_traits::allocate(_allocator, _capacity));
        std::uninitialized_copy_n(o._data, _size, _data);
        return *this;
    }

    /// \brief Constructs a new stack using move semantics.
    /// This function fails only when other object is invalid.
    /// \exception ::InvalidStateError Argument was invalid.
    Stack(Stack &&o) {
        if (!o.valid())
            throw StackInvalidState{};

        set_data(std::exchange(o._data, nullptr));
        set_capacity(std::exchange(o._capacity, 0));
        set_size(std::exchange(o._size, 1));
        _allocator = std::move(o._allocator);
    }

    /// \brief Replaces current stack with the new stack using move semantics.
    /// This functions fails only
    /// \exception ::InvalidStateError At least one operand was invalid.
    Stack &operator=(Stack &&o) {
        if (this == &o)
            return *this;

        clear();
        if (!o.valid())
            throw StackInvalidState{};

        set_data(std::exchange(o._data, nullptr));
        set_capacity(std::exchange(o._capacity, 0));
        set_size(std::exchange(o._size, 1));
        _allocator = std::move(o._allocator);

        assert(valid());
        return *this;
    }

    ~Stack() {
        // I can't do anything reasonable here if stack was broken
        clear_internal();
    }

    void push(const T &elem) { return emplace(elem); }

    void push(T &&elem) { return emplace(std::move(elem)); }

    template <class... Args>
    void emplace(Args &&... args) {
        if (!valid())
            throw StackInvalidState{};

        if (_size == _capacity)
            reserve(_capacity * growth_factor + 1);

        allocator_traits::construct(_allocator, _data + _size,
                                    std::forward<Args>(args)...);
        set_size(_size + 1);

        assert(valid());
    }

    void pop() {
        if (!valid())
            throw StackInvalidState{};

        if (_size == 0)
            throw StackUnderflow{};

        set_size(_size - 1);
        allocator_traits::destroy(_allocator, _data + _size);
        if ((double)_size / _capacity < shrink_factor)
            reserve(_size);

        assert(valid());
    }

    T &top() {
        if (!valid())
            throw StackInvalidState{};

        if (_size == 0)
            throw StackUnderflow{};

        assert(valid());
        return _data[_size - 1];
    }

    const T &top() const noexcept { return top(); };

    /// \brief Allocates memory to store at least `new_capacity` elements
    /// without reallocation This function may fail in any of these cases:
    ///
    void reserve(std::size_t new_capacity) {
        if (!valid())
            throw StackInvalidState{};

        if (new_capacity == 0)
            return clear_internal();

        auto new_size = std::min(_capacity, _size);
        auto new_data =
            allocator_traits::allocate(_allocator, new_capacity, _data);
        if (_data != nullptr) {
            std::uninitialized_move_n(_data, new_size, new_data);
            std::destroy_n(_data, _size);
            allocator_traits::deallocate(_allocator, _data, _capacity);
        }
        set_capacity(new_capacity);
        set_size(new_size);
        set_data(new_data);
        assert(valid());
    }

    /// \brief Returns the stack to initial empty state.
    ///
    /// This function may fail in any of these cases:
    /// 1. Internal representation was corrupted;
    /// 2. Elements' destructors throw exception;
    /// 3. Deallocation throws exception (somehow).
    /// In other scenarios function should not fail.
    void clear() {
        if (!valid())
            throw StackInvalidState{};

        clear_internal();
        assert(valid());
    }

    /// \brief Returns a number of elements in the stack.
    /// \return a number of elements in the stack.
    std::size_t size() const {
        if (!valid())
            throw StackInvalidState{};

        assert(valid());
        return _size;
    };

    /// \brief Checks if the stack is empty.
    /// Stack is empty when it's size is 0.
    /// \return if the stack is empty
    inline bool empty() const {
        if (!valid())
            throw StackInvalidState{};

        assert(valid());
        return _size == 0;
    };

    /// \brief Checks if the stack' internal representation is valid.
    /// Stack is valid if all these conditions holds:
    /// 1. \f$size \le capacity\f$
    /// 2. \f$capacity = 0 \Leftrightarrow data = \text{nullptr}\f$
    /// \return if the stack is valid.
    inline bool valid() const {
        return _checksum == calculate_checksum() && _size <= _capacity &&
               ((_capacity == 0 && _data == nullptr) ||
                (_capacity != 0 && _data != nullptr));
    }

private:
    using allocator_traits = std::allocator_traits<Allocator>;

    const double growth_factor = 2;
    const double shrink_factor = 0.4;

    T *_data{nullptr};
    std::size_t _capacity{0};
    std::size_t _size{0};
    Allocator _allocator;
    unsigned long long _checksum{calculate_checksum()};

    void clear_internal() {
        std::destroy_n(_data, _size);
        allocator_traits::deallocate(_allocator, _data, _capacity);
        set_data(nullptr);
        set_capacity(0);
        set_size(0);
        assert(valid());
    }

    inline void set_data(const decltype(_data) data) {
        _data = data;
        _checksum = calculate_checksum();
    }

    inline void set_capacity(const decltype(_capacity) capacity) {
        _capacity = capacity;
        _checksum = calculate_checksum();
    }

    inline void set_size(const decltype(_size) size) {
        _size = size;
        _checksum = calculate_checksum();
    }

    inline unsigned long long calculate_checksum() const {
        using Ret = decltype(calculate_checksum());
        Ret res = 1;
        res = 31 * res + reinterpret_cast<std::uintptr_t>(_data);
        res = 31 * res + _capacity;
        res = 31 * res + _size;
        return res;
    }
};

} // namespace safe_stack

#endif // SAFE_STACK_H
