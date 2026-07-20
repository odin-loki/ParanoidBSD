module;
#include <cstdint>

export module pbsd.kernel.vm_reserv;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/vm/vm_reserv.c — address reservation helpers.
export namespace pbsd::kernel::vm_reserv {

struct Reservation {
    std::uint64_t start{};
    std::uint64_t size{};
    bool          active{false};
};

[[nodiscard]] inline Status reserve(Reservation& r, std::uint64_t start,
                                    std::uint64_t size) noexcept {
    if (size == 0 || start + size < start) {
        return Status::Invalid;
    }
    if (r.active) {
        return Status::Busy;
    }
    r.start = start;
    r.size = size;
    r.active = true;
    return Status::Ok;
}

[[nodiscard]] inline Status release(Reservation& r) noexcept {
    if (!r.active) {
        return Status::NotFound;
    }
    r.active = false;
    return Status::Ok;
}

[[nodiscard]] inline bool contains(Reservation const& r,
                                   std::uint64_t addr) noexcept {
    return r.active && addr >= r.start && addr < r.start + r.size;
}

} // namespace pbsd::kernel::vm_reserv
