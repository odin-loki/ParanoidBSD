module;
#include <cstdint>

export module pbsd.kernel.vm_page;

import pbsd.core;

/// Freestanding port of `vm_page.h` queue indices + page state helpers.
export namespace pbsd::kernel::vm_page {

inline constexpr unsigned kPqNone       = 255u;
inline constexpr unsigned kPqInactive  = 0u;
inline constexpr unsigned kPqActive    = 1u;
inline constexpr unsigned kPqLaundry   = 2u;
inline constexpr unsigned kPqUnswappable = 3u;
inline constexpr unsigned kPqCount     = 4u;

inline constexpr unsigned kVmPageAflagShift = 16u;

struct PageQueueEntry {
    unsigned queue;
    unsigned flags;
};

[[nodiscard]] constexpr bool is_active_queue(unsigned q) noexcept {
    return q == kPqActive;
}

[[nodiscard]] constexpr bool is_inactive_queue(unsigned q) noexcept {
    return q == kPqInactive;
}

[[nodiscard]] constexpr bool is_laundry_family(unsigned q) noexcept {
    return q == kPqLaundry || q == kPqUnswappable;
}

[[nodiscard]] constexpr bool is_on_queue(unsigned q) noexcept {
    return q != kPqNone;
}

/// `vm_page_queue()` — extract queue index from page state.
[[nodiscard]] constexpr unsigned page_queue(unsigned encoded_queue) noexcept {
    return encoded_queue;
}

/// `vm_page_aflag_set` — set advisory flags in high bits.
[[nodiscard]] constexpr unsigned set_aflags(unsigned base, unsigned bits) noexcept {
    return base | (bits << kVmPageAflagShift);
}

/// `vm_page_all_valid` — compare valid field against VM_PAGE_BITS_ALL (8-bit).
[[nodiscard]] constexpr bool all_valid(unsigned char valid, unsigned char bits_all) noexcept {
    return valid == bits_all;
}

inline constexpr unsigned char kVmPageBitsAll8 = 0xffu;

struct PageCounts {
    unsigned active{};
    unsigned inactive{};
    unsigned laundry{};
    unsigned free_min{};
};

/// Pageout target policy from `vm_pageout.c` free-page watermarks.
[[nodiscard]] inline Status evaluate_free_target(const PageCounts& cnt,
                                                 unsigned target,
                                                 unsigned min) noexcept {
    if (cnt.active + cnt.inactive + cnt.laundry < min) {
        return Status::Busy;
    }
    if (cnt.inactive >= target) {
        return Status::Ok;
    }
    return Status::Protocol;
}

} // namespace pbsd::kernel::vm_page
