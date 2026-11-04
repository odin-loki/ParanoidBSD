module;

export module pbsd.kernel.subr_filter;

import pbsd.core;

/// Freestanding port of `kern/subr_filter.c` — time filter setup helpers.
export namespace pbsd::kernel::subr_filter {

inline constexpr int kTypeMin = 1;
inline constexpr int kTypeMax = 2;

[[nodiscard]] inline Status setup_type(int fil_type) noexcept {
    if (fil_type != kTypeMin && fil_type != kTypeMax) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_time_len(unsigned time_len) noexcept {
    if (time_len == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::subr_filter
