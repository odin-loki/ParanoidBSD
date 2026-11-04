module;
#include <cstdint>

export module pbsd.kernel.vm_phys;

import pbsd.core;

/// Freestanding port of `vm_phys.c` / `_vm_phys.h` free-list order helpers.
export namespace pbsd::kernel::vm_phys {

inline constexpr unsigned kVmNfreeorderMax = 12u; // VM_NFREEORDER typical amd64

struct PhysSeg {
    std::uint64_t start{};
    std::uint64_t end{};
    unsigned      domain{};
};

[[nodiscard]] constexpr std::uint64_t page_index(std::uint64_t pa,
                                                 std::uint64_t seg_start) noexcept {
    return (pa - seg_start) >> 12;
}

[[nodiscard]] constexpr unsigned free_order_for_pages(unsigned pages) noexcept {
    unsigned order = 0;
    unsigned p = pages;
    while (p > 1 && order < kVmNfreeorderMax) {
        p >>= 1;
        ++order;
    }
    return order;
}

[[nodiscard]] constexpr Status validate_phys_range(const PhysSeg& seg) noexcept {
    if (seg.end <= seg.start) {
        return Status::Invalid;
    }
    if ((seg.start & 0xfffu) != 0 || (seg.end & 0xfffu) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

/// Buddy coalesce — adjacent same-order blocks merge if addresses align.
[[nodiscard]] constexpr bool buddy_mergeable(std::uint64_t a, std::uint64_t b,
                                             unsigned order) noexcept {
    const std::uint64_t mask = (1ull << (order + 12)) - 1;
    return ((a ^ b) & ~mask) == 0;
}

} // namespace pbsd::kernel::vm_phys
