module;
#include <cstdint>

export module pbsd.kernel.vm_meter;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/vm/vm_meter.c — vm statistics helpers.
export namespace pbsd::kernel::vm_meter {

struct Counters {
    std::uint64_t page_count{};
    std::uint64_t free_count{};
    std::uint64_t active_count{};
    std::uint64_t inactive_count{};
    std::uint64_t wired_count{};
};

[[nodiscard]] inline Status init(Counters& c) noexcept {
    c = Counters{};
    return Status::Ok;
}

[[nodiscard]] inline std::uint64_t used_pages(Counters const& c) noexcept {
    return c.active_count + c.inactive_count + c.wired_count;
}

[[nodiscard]] inline bool pressure(Counters const& c,
                                   std::uint64_t threshold) noexcept {
    if (c.page_count == 0) {
        return false;
    }
    return c.free_count * 100 / c.page_count < threshold;
}

} // namespace pbsd::kernel::vm_meter
