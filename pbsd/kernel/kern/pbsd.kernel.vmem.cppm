module;
#include <cstdint>

export module pbsd.kernel.vmem;

export import pbsd.core;

/// Freestanding port of `sys/vmem.h` / `kern/subr_vmem.c`.
export namespace pbsd::kernel::vmem {

inline constexpr std::uint64_t kAddrMin = 0;
inline constexpr std::uint64_t kAddrQcacheMin = 1;

enum class OpFlag : unsigned {
    Alloc = 0x01,
    Free = 0x02,
    MaxFree = 0x10,
};

enum class FitFlag : unsigned {
    First = 0x1000,
    Best = 0x2000,
    Next = 0x8000,
};

[[nodiscard]] inline Status validate_range(std::uint64_t start, std::uint64_t size) noexcept {
    if (size == 0) {
        return Status::Invalid;
    }
    if (start < kAddrMin) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::vmem
