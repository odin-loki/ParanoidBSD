module;
#include <cstdint>

export module pbsd.kernel.sbuf;

export import pbsd.core;

/// Freestanding port concepts from `sys/sbuf.h` / `kern/subr_sbuf.c`.
export namespace pbsd::kernel::sbuf {

inline constexpr unsigned kDefaultCapacity = 256;
inline constexpr unsigned kMaxCapacity = 1u << 20;

enum class Flag : unsigned {
    None = 0,
    Fixed = 1u << 0,
    Dynamic = 1u << 1,
    Finished = 1u << 2,
    Overflow = 1u << 3,
};

struct Softc {
    char* data{nullptr};
    unsigned length{0};
    unsigned capacity{0};
    Flag flags{Flag::None};
};

[[nodiscard]] constexpr Flag operator|(Flag a, Flag b) noexcept {
    return static_cast<Flag>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}

[[nodiscard]] constexpr bool has(Flag f, Flag bit) noexcept {
    return (static_cast<unsigned>(f) & static_cast<unsigned>(bit)) != 0;
}

[[nodiscard]] inline Status validate_capacity(unsigned cap) noexcept {
    if (cap == 0 || cap > kMaxCapacity) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status append_len(Softc& sb, unsigned n) noexcept {
    if (has(sb.flags, Flag::Finished)) {
        return Status::Denied;
    }
    if (sb.length + n > sb.capacity) {
        sb.flags = sb.flags | Flag::Overflow;
        return Status::NoMemory;
    }
    sb.length += n;
    return Status::Ok;
}

[[nodiscard]] inline Status finish(Softc& sb) noexcept {
    sb.flags = sb.flags | Flag::Finished;
    return Status::Ok;
}

} // namespace pbsd::kernel::sbuf
