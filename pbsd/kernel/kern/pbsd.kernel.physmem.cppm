module;
#include <cstdint>

export module pbsd.kernel.physmem;

export import pbsd.core;

/// Freestanding port of `sys/physmem.h` / `kern/subr_physmem.c`.
export namespace pbsd::kernel::physmem {

inline constexpr unsigned kExflagNoDump = 0x01;
inline constexpr unsigned kExflagNoAlloc = 0x02;

struct Region {
    std::uint64_t start{};
    std::uint64_t size{};
    unsigned flags{};
};

[[nodiscard]] inline Status validate_region(const Region& r) noexcept {
    if (r.size == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool excluded(const Region& r, unsigned flag) noexcept {
    return (r.flags & flag) != 0;
}

} // namespace pbsd::kernel::physmem
