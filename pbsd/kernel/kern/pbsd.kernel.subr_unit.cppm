module;

export module pbsd.kernel.subr_unit;

import pbsd.core;

/// Freestanding port of `subr_unit.c` — unit number allocation header.
export namespace pbsd::kernel::subr_unit {

inline constexpr int kNoUnit = -1;

struct UnitRange {
    int low{0};
    int high{0};
    unsigned allocated{0};
};

[[nodiscard]] inline Status init(UnitRange& r, int max) noexcept {
    if (max <= 0) {
        return Status::Invalid;
    }
    r.low = 0;
    r.high = max;
    r.allocated = 0;
    return Status::Ok;
}

[[nodiscard]] inline int alloc(UnitRange& r) noexcept {
    if (r.allocated >= static_cast<unsigned>(r.high - r.low)) {
        return kNoUnit;
    }
    return r.low + static_cast<int>(r.allocated++);
}

[[nodiscard]] inline Status free_unit(UnitRange& r, int unit) noexcept {
    if (unit < r.low || unit >= r.high) {
        return Status::Invalid;
    }
    if (r.allocated > 0) {
        --r.allocated;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::subr_unit
