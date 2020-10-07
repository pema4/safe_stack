#ifndef SAFE_STACK_H
#define SAFE_STACK_H

#include "expected.h"
#include <functional>
#include <memory>

namespace safe_stack {

enum class StackError { UNDERFLOW, BAD_ALLOC, INVALID_STATE };

/// \brief Safe stack class.
/// Main design decisions (maybe they are wrong, I am not sure):
/// 1. No exceptions is thrown by stack. Instead every method returns Expected
/// ::Expected<R, E> value, which holds either the error or the result.
/// Due to [[nodiscard]] attributes compiler yells at users if they don't check
/// operation result.
/// 2. Copy constructor and assignment are deleted, because they need to
/// reallocate memory, and this can cause exceptions. Todo: fix this
/// 3. If object is moved out to a new place, it is marked as invalid
/// (`size` becomes bigger than `capacity). Any operation on invalid object
/// returns ::StackError::INVALID_STATE
///
template <class T, class Allocator = std::allocator<T>>
class Stack {
public:
    /// \brief Constructs an empty stack.
    /// This function doesn't allocate any memory, so it never fails.
    Stack() noexcept = default;

    /// \brief Copy construction mechanism is deleted.
    /// TOdo:
    Stack(const Stack &);

    Stack &operator=(const Stack &other);

    /// \brief Constructs a new stack using move semantics.
    /// This function doesn't allocate any memory, so it never fails.
    Stack(Stack &&other) noexcept;

    /// \brief Replaces current stack with the new stack using move semantics.
    /// This function destroys old objects and deallocates previously allocated
    /// memory, so it can fail (but `Stack` is not responsible for it).
    Stack &operator=(Stack &&other);

    ~Stack();

    [[nodiscard]] ExpectedVoid<StackError> push(const T &elem);

    [[nodiscard]] ExpectedVoid<StackError> push(T &&elem);

    template <class... Args>
    [[nodiscard]] ExpectedVoid<StackError> emplace(Args &&... args);

    [[nodiscard]] ExpectedVoid<StackError> pop();

    [[nodiscard]] Expected<std::reference_wrapper<T>, StackError>
    top() noexcept;

    [[nodiscard]] Expected<std::reference_wrapper<const T>, StackError>
    top() const noexcept;

    [[nodiscard]] ExpectedVoid<StackError> reserve(std::size_t new_capacity);

    void clear();

    /// \brief Returns a number of elements in the stack.
    /// \return a number of elements in the stack.
    inline std::size_t size() const noexcept { return _size; };

    /// \brief Checks if the stack is empty.
    /// Stack is empty when it's size is 0.
    /// \return if the stack is empty
    inline bool empty() const noexcept { return _size == 0; };

    /// \brief Checks if the stack' internal representation is valid.
    /// Stack is valid if all these conditions holds:
    /// 1. \f$size \le capacity\f$
    /// 2. \f$capacity = 0 \Leftrightarrow data = \text{nullptr}\f$
    /// \return if the stack is valid.
    bool is_valid() const;

private:
    using allocator_traits = std::allocator_traits<Allocator>;

    const double growth_factor = 2;
    const double shrink_factor = 0.4;

    T *_data{nullptr};
    std::size_t _capacity{0};
    std::size_t _size{0};
    Allocator _allocator;
};

template <class T, class A>
Stack<T, A>::Stack(const Stack &o)
    : _capacity{o._size}, _size{o._size}, _allocator{o._allocator} {
    _data = allocator_traits::allocate(_allocator, _capacity);
    std::uninitialized_copy_n(o._data, _size, _data);
}

template <class T, class A>
Stack<T, A> &Stack<T, A>::operator=(const Stack &o) {
    if (this == &o)
        return *this;

    _capacity = o._capacity;
    _size = o._size;
    _allocator = o._allocator;
    _data = allocator_traits::allocate(_allocator, _capacity);
    std::uninitialized_copy_n(o._data, _size, _data);
    return *this;
}

template <class T, class A>
Stack<T, A>::Stack(Stack &&o) noexcept
    : _data{std::exchange(o._data, nullptr)}, _capacity{std::exchange(
                                                  o._capacity, 0)},
      _size{std::exchange(o._size, 1)}, _allocator{o._allocator} {}

template <class T, class A>
Stack<T, A> &Stack<T, A>::operator=(Stack &&o) {
    if (this == &o)
        return *this;

    // I guess I need to deallocate previously taken memory there
    clear();
    _data = std::exchange(o._data, nullptr);
    _capacity = std::exchange(o._capacity, 0);
    _size = std::exchange(o._size, 0);
    _allocator = std::move(o._allocator);
    return *this;
}

template <class T, class A>
[[nodiscard]] ExpectedVoid<StackError> Stack<T, A>::push(const T &elem) {
    return emplace(elem);
}

template <class T, class A>
[[nodiscard]] ExpectedVoid<StackError> Stack<T, A>::push(T &&elem) {
    return emplace(std::move(elem));
}

template <class T, class A>
template <class... Args>
[[nodiscard]] ExpectedVoid<StackError> Stack<T, A>::emplace(Args &&... args) {
    if (!is_valid())
        return StackError::INVALID_STATE;
    if (_size == _capacity) {
        auto res = reserve(_capacity * growth_factor + 1);
        if (res.has_error())
            return res;
    }
    allocator_traits::construct(_allocator, _data + _size,
                                std::forward<Args>(args)...);
    _size += 1;
    return {};
}

template <class T, class A>
[[nodiscard]] ExpectedVoid<StackError> Stack<T, A>::pop() {
    if (!is_valid())
        return StackError::INVALID_STATE;
    if (_size == 0 || _data == nullptr)
        return StackError::UNDERFLOW;
    _size -= 1;
    allocator_traits::destroy(_allocator, _data + _size);
    if ((double)_size / _capacity < shrink_factor) {
        auto res = reserve(_size);
        if (res.has_error())
            return res;
    }
    return {};
}

template <class T, class A>
[[nodiscard]] Expected<std::reference_wrapper<T>, StackError>
Stack<T, A>::top() noexcept {
    if (!is_valid())
        return StackError::INVALID_STATE;
    if (_size == 0)
        return StackError::UNDERFLOW;
    return std::ref(_data[_size - 1]);
}

template <class T, class A>
[[nodiscard]] Expected<std::reference_wrapper<const T>, StackError>
Stack<T, A>::top() const noexcept {
    if (!is_valid())
        return StackError::INVALID_STATE;
    auto top_res = top();
    if (top_res.is_error())
        return top_res;
    return std::cref(top_res.result());
};

template <class T, class A>
[[nodiscard]] ExpectedVoid<StackError>
Stack<T, A>::reserve(std::size_t new_capacity) {
    if (!is_valid())
        return StackError::INVALID_STATE;
    try {
        auto new_data =
            allocator_traits::allocate(_allocator, new_capacity, _data);
        if (_data != nullptr) {
            std::uninitialized_move_n(_data, _capacity, new_data);
            std::destroy_n(_data, _size);
            allocator_traits::deallocate(_allocator, _data, _capacity);
        }
        _capacity = new_capacity;
        _data = new_data;
    } catch (std::bad_alloc &ex) {
        return StackError::BAD_ALLOC;
    }
    return {};
}

template <class T, class A>
void Stack<T, A>::clear() {
    if (is_valid()) {
        std::destroy_n(_data, _size);
        allocator_traits::deallocate(_allocator, _data, _capacity);
        _data = nullptr;
    }
}

template <class T, class A>
Stack<T, A>::~Stack() {
    clear();
}

template <class T, class A>
bool Stack<T, A>::is_valid() const {
    return _size <= _capacity && ((_capacity == 0 && _data == nullptr) ||
                                  (_capacity != 0 && _data != nullptr));
}

} // namespace safe_stack

#endif // SAFE_STACK_H
