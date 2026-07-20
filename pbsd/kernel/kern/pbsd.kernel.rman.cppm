module;
#include <cstdint>

export module pbsd.kernel.rman;

export import pbsd.core;

/// Freestanding port of `sys/rman.h` / `kern/subr_rman.c`.
export namespace pbsd::kernel::rman {

inline constexpr unsigned kShareable = 0x01;
inline constexpr unsigned kSparse = 0x02;

struct Region {
    std::uint64_t start{};
    std::uint64_t end{};
    unsigned flags{};
};

[[nodiscard]] inline Status validate_region(const Region& r) noexcept {
    if (r.end < r.start) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline std::uint64_t size(const Region& r) noexcept {
    return r.end - r.start + 1;
}

} // namespace pbsd::kernel::rman
