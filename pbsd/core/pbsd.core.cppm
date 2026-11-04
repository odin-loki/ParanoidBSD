export module pbsd.core;

export namespace pbsd {

enum class Status : int {
    Ok = 0,
    Invalid = 1,
    Denied = 2,
    Timeout = 3,
    NoMemory = 4,
    NotFound = 5,
    Busy = 6,
    Protocol = 7,
    NotImplemented = 8,
};

[[nodiscard]] constexpr bool ok(Status s) noexcept { return s == Status::Ok; }

struct Void {};

template<typename T>
struct Result {
    Status status{Status::Ok};
    T value{};

    [[nodiscard]] constexpr bool has_value() const noexcept { return status == Status::Ok; }
    [[nodiscard]] constexpr explicit operator bool() const noexcept { return has_value(); }

    [[nodiscard]] constexpr T unwrap_or(T fallback) const noexcept {
        return has_value() ? value : fallback;
    }
};

template<>
struct Result<Void> {
    Status status{Status::Ok};

    [[nodiscard]] constexpr bool has_value() const noexcept { return status == Status::Ok; }
    [[nodiscard]] constexpr explicit operator bool() const noexcept { return has_value(); }
};

using StatusOnly = Result<Void>;

[[nodiscard]] constexpr StatusOnly status_ok() noexcept { return StatusOnly{Status::Ok}; }
[[nodiscard]] constexpr StatusOnly status_err(Status s) noexcept { return StatusOnly{s}; }

template<typename T>
[[nodiscard]] constexpr Result<T> result_ok(T value) noexcept {
    return Result<T>{Status::Ok, value};
}

template<typename T>
[[nodiscard]] constexpr Result<T> result_err(Status s, T value = {}) noexcept {
    return Result<T>{s, value};
}

/// Freestanding-friendly utilities (no std dependency).
namespace util {

template<typename T>
[[nodiscard]] constexpr T&& forward(T& t) noexcept {
    return static_cast<T&&>(t);
}

template<typename T>
[[nodiscard]] constexpr T&& forward(T&& t) noexcept {
    return static_cast<T&&>(t);
}

template<typename T>
[[nodiscard]] constexpr T move(T& t) noexcept {
    return static_cast<T&&>(t);
}

template<typename T>
constexpr void swap(T& a, T& b) noexcept {
    T tmp = move(a);
    a = move(b);
    b = move(tmp);
}

template<typename T>
[[nodiscard]] constexpr T exchange(T& obj, T new_value) noexcept {
    T old = move(obj);
    obj = move(new_value);
    return old;
}

template<typename T>
[[nodiscard]] constexpr const T& min(const T& a, const T& b) noexcept {
    return (b < a) ? b : a;
}

template<typename T>
[[nodiscard]] constexpr const T& max(const T& a, const T& b) noexcept {
    return (a < b) ? b : a;
}

template<typename T>
[[nodiscard]] constexpr T clamp(T v, T lo, T hi) noexcept {
    return max(lo, min(v, hi));
}

} // namespace util

} // namespace pbsd
