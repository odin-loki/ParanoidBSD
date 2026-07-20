module;

export module pbsd.kernel.unit;

export import pbsd.core;
export import pbsd.kernel.subr_unit;

/// Freestanding port of `sys/unit.h` — device unit number helpers.
export namespace pbsd::kernel::unit {

using subr_unit::UnitRange;
using subr_unit::kNoUnit;

inline constexpr int kWildcard = -1;

[[nodiscard]] inline Status validate_unit(int unit, int max) noexcept {
    if (unit < 0 || unit >= max) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool is_wildcard(int unit) noexcept {
    return unit == kWildcard;
}

[[nodiscard]] inline Status init_range(UnitRange& r, int max) noexcept {
    return subr_unit::init(r, max);
}

} // namespace pbsd::kernel::unit
