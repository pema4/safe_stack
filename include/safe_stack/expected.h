#ifndef EXPECTED_H
#define EXPECTED_H

#include <variant>

template <class R, class E>
class Expected {
public:
    constexpr Expected() = default;
    constexpr Expected(const Expected<R, E> &) = default;
    constexpr Expected(Expected<R, E> &&) = default;
    constexpr Expected<R, E> &operator=(const Expected<R, E> &) = default;
    constexpr Expected<R, E> &operator=(Expected<R, E> &&) = default;
    constexpr Expected(E &&err) : data(err) {}
    constexpr Expected(R &&result) : data(result) {}

    constexpr bool has_error() const { return std::holds_alternative<E>(data); }

    constexpr E error() const { return std::get<E>(data); }

    constexpr bool has_result() const {
        return std::holds_alternative<R>(data);
    }

    constexpr R result() const { return std::get<R>(data); }

    constexpr operator bool() const { return has_error(); }

private:
    std::variant<R, E> data;
};

template <typename E>
using ExpectedVoid = Expected<std::monostate, E>;

#endif // EXPECTED_H
