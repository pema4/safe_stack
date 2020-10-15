#ifndef SAFE_STACK_H
#define SAFE_STACK_H

#include "safe_stack/cache.h"
#include <cassert> // for assert
#include <cstdint> // for std::uintptr_t
#include <iostream>
#include <memory>

namespace safe_stack {

struct StackError {};

struct StackUnderflow : public StackError {};

struct StackInvalidState : public StackError {};

/// \brief Safe stack class.
/// Main design decisions:
/// 1. Every operation can throw ::StackError.
/// 2. Stack calculates checksum of its fields and checks it before every
/// operation.
/// 3. If object is moved out to a new place, it is marked as invalid
/// (`size` becomes bigger than `capacity`). Any operation on invalid object
/// throws ::StackInvalidState
///
template <class T, class Allocator = std::allocator<T>>
class Stack {
public:
    /// \brief Constructs an empty stack.
    /// This function never fails.
    Stack() noexcept;

    /// \brief Constructs a copy of the stack.
    /// \exception ::InvalidStateError Argument was invalid.
    Stack(const Stack &o);

    /// \brief Copies the stack to this stack.
    /// \exception ::InvalidStateError Argument was invalid.
    Stack &operator=(const Stack &o);

    /// \brief Constructs a copy of the stack using move constructor.
    /// \exception ::InvalidStateError Argument was invalid.
    Stack(Stack &&o);

    /// \brief Moves the stack to this stack.
    /// \exception ::InvalidStateError Argument was invalid.
    Stack &operator=(Stack &&o);

    /// \brief Stack destructor
    ~Stack();

    void push(const T &elem);

    void push(T &&elem);

    template <class... Args>
    void emplace(Args &&... args);

    void pop();

    T &top();

    const T &top() const;

    /// \brief Allocates memory to store at least `new_capacity` elements
    /// without reallocation This function may fail in any of these cases:
    ///
    void reserve(std::size_t new_capacity);

    /// \brief Returns the stack to initial empty state.
    ///
    /// This function may fail in any of these cases:
    /// 1. Internal representation was corrupted;
    /// 2. Elements' destructors throw exception;
    /// 3. Deallocation throws exception (somehow).
    /// In other scenarios function should not fail.
    void clear();

    /// \brief Returns a number of elements in the stack.
    /// \return a number of elements in the stack.
    std::size_t size() const;

    /// \brief Checks if the stack is empty.
    /// Stack is empty when it's size is 0.
    /// \return if the stack is empty
    inline bool empty() const;

    /// \brief Checks if the stack' internal representation is valid.
    /// Stack is valid if all these conditions holds:
    /// 1. \f$size \le capacity\f$
    /// 2. \f$capacity = 0 \Leftrightarrow data = \text{nullptr}\f$
    /// \return if the stack is valid.
    inline bool valid() const;

    template <class T2, class A2>
    friend std::ostream &operator<<(std::ostream &out,
                                    const Stack<T2, A2> &stack);

private:
    using allocator_traits = std::allocator_traits<Allocator>;

    static constexpr double growth_factor = 2;
    static constexpr double shrink_factor = 0.4;
    static constexpr unsigned long long canary_value = 0xDEADBEEFBADF00Dul;

    decltype(canary_value) start_canary{canary_value};
    T *_data{nullptr}; // todo: add guards to data
    std::size_t _capacity{0};
    mutable CacheType _cache{0};
    std::size_t _size{0};
    Allocator _allocator;
    decltype(canary_value) end_canary{canary_value};

    void clear_internal();

    void validate() const;

    CacheType compute_cache() const;
};

template <class T, class A>
std::ostream &operator<<(std::ostream &out, const Stack<T, A> &stack) {
    out << "Stack capacity: " << stack._capacity << " size: " << stack._size
        << " {";
    for (auto i = 0u; i < stack._capacity; ++i) {
        out << "  [" << i << "] = ";
        if (i < stack._size)
            out << stack._data[i];
        else
            out << "GARBAGE";
        out << "\n";
    }
    return out;
}

template <class T, class A>
Stack<T, A>::Stack() noexcept {
    _cache = compute_cache();
    validate();
}

template <class T, class A>
Stack<T, A>::Stack(const Stack &o) {
    o.validate();

    _capacity = o._capacity;
    _size = o._size;
    _allocator = o._allocator;
    _data = allocator_traits::allocate(_allocator, _capacity);
    _cache = compute_cache();
    std::uninitialized_copy_n(o._data, _size, _data);

    validate();
}

template <class T, class A>
Stack<T, A> &Stack<T, A>::operator=(const Stack &o) {
    if (this == &o)
        return *this;

    validate();
    o.validate();

    _capacity = o._capacity;
    _size = o._size;
    _allocator = o._allocator;
    _data = allocator_traits::allocate(_allocator, _capacity);
    _cache = compute_cache();
    std::uninitialized_copy_n(o._data, _size, _data);

    validate();
    return *this;
}

template <class T, class A>
Stack<T, A>::Stack(Stack &&o) {
    o.validate();

    _data = std::exchange(o._data, nullptr);
    _capacity = std::exchange(o._capacity, 0);
    _size = std::exchange(o._size, 1);
    _allocator = std::move(o._allocator);
    _cache = compute_cache();

    validate();
}

template <class T, class A>
Stack<T, A> &Stack<T, A>::operator=(Stack &&o) {
    if (this == &o)
        return *this;

    validate();
    o.validate();
    clear();

    _data = std::exchange(o._data, nullptr);
    _capacity = std::exchange(o._capacity, 0);
    _size = std::exchange(o._size, 1);
    _allocator = std::move(o._allocator);
    _cache = compute_cache();

    validate();
    return *this;
}

template <class T, class A>
Stack<T, A>::~Stack() {
    if (valid())
        clear_internal();
}

template <class T, class A>
void Stack<T, A>::push(const T &elem) {
    return emplace(elem);
}

template <class T, class A>
void Stack<T, A>::push(T &&elem) {
    return emplace(std::move(elem));
}

template <class T, class A>
template <class... Args>
void Stack<T, A>::emplace(Args &&... args) {
    validate();

    if (_size == _capacity)
        reserve(_capacity * growth_factor + 1);

    allocator_traits::construct(_allocator, _data + _size,
                                std::forward<Args>(args)...);
    _size = _size + 1;
    _cache = compute_cache();
    validate();
}

template <class T, class A>
void Stack<T, A>::pop() {
    validate();
    if (_size == 0)
        throw StackUnderflow{};

    _size = _size - 1;
    _cache = compute_cache();
    allocator_traits::destroy(_allocator, _data + _size);
    if ((double)_size / _capacity < shrink_factor)
        reserve(_size);

    validate();
}

template <class T, class A>
T &Stack<T, A>::top() {
    validate();
    if (_size == 0)
        throw StackUnderflow{};

    return _data[_size - 1];
}

template <class T, class A>
const T &Stack<T, A>::top() const {
    return top();
};

template <class T, class A>
void Stack<T, A>::reserve(std::size_t new_capacity) {
    validate();
    if (new_capacity == 0)
        return clear_internal();

    auto new_size = std::min(_capacity, _size);
    auto new_data = allocator_traits::allocate(_allocator, new_capacity, _data);
    if (_data != nullptr) {
        std::uninitialized_move_n(_data, new_size, new_data);
        std::destroy_n(_data, _size);
        allocator_traits::deallocate(_allocator, _data, _capacity);
    }
    _capacity = new_capacity;
    _size = new_size;
    _data = new_data;
    _cache = compute_cache();
    validate();
}

template <class T, class A>
void Stack<T, A>::clear() {
    validate();
    clear_internal();
    validate();
}

template <class T, class A>
std::size_t Stack<T, A>::size() const {
    validate();
    return _size;
};

template <class T, class A>
inline bool Stack<T, A>::empty() const {
    return size() == 0;
};

template <class T, class A>
inline bool Stack<T, A>::valid() const {
    return start_canary == canary_value && end_canary == canary_value &&
           _cache == compute_cache() && _size <= _capacity &&
           ((_capacity == 0 && _data == nullptr) ||
            (_capacity != 0 && _data != nullptr));
}

template <class T, class A>
void Stack<T, A>::clear_internal() {
    if (_data != nullptr) {
        std::destroy_n(_data, _size);
        allocator_traits::deallocate(_allocator, _data, _capacity);
        _data = nullptr;
        _capacity = 0;
        _size = 0; // stack becomes invalid if size > capacity
        _cache = compute_cache();
    }
    validate();
}

template <class T, class A>
inline void Stack<T, A>::validate() const {
    if (!valid()) {
        std::cerr << *this;
        throw StackInvalidState{};
    }
}

template <class T, class A>
CacheType Stack<T, A>::compute_cache() const {
    auto old_cache = _cache;
    _cache = 0;
    auto result = cache(*this);
    _cache = old_cache;
    return result;
}

} // namespace safe_stack

#endif // SAFE_STACK_H
