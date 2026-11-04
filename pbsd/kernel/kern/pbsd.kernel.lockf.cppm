module;
#include <cstdint>

export module pbsd.kernel.lockf;

export import pbsd.core;

/// Wave 5 — advisory lock flags from sys/lockf.h, sys/fcntl.h.
export namespace pbsd::kernel::lockf {

inline constexpr short kFIntr  = static_cast<short>(0x8000);
inline constexpr short kFRdLck = 1;
inline constexpr short kFUnLck = 2;
inline constexpr short kFWrLck = 3;
inline constexpr short kFPosix = static_cast<short>(0x040);

struct LockfEntry {
    short     flags{};
    short     type{};
    std::int64_t start{};
    std::int64_t end{};
};

[[nodiscard]] constexpr bool is_posix(short flags) noexcept {
    return (flags & kFPosix) != 0;
}

[[nodiscard]] constexpr bool is_read_lock(short type) noexcept {
    return type == kFRdLck;
}

[[nodiscard]] constexpr bool is_write_lock(short type) noexcept {
    return type == kFWrLck;
}

[[nodiscard]] constexpr Status validate_range(std::int64_t start, std::int64_t end) noexcept {
    if (start < 0 || end < start) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr Status validate_entry(const LockfEntry& e) noexcept {
    if (e.type != kFRdLck && e.type != kFWrLck && e.type != kFUnLck) {
        return Status::Invalid;
    }
    return validate_range(e.start, e.end);
}

} // namespace pbsd::kernel::lockf
