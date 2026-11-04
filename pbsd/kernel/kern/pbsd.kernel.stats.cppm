module;

export module pbsd.kernel.stats;

export import pbsd.core;

/// Freestanding port of `sys/stats.h` / `kern/subr_stats.c`.
export namespace pbsd::kernel::stats {

inline constexpr unsigned kTplMaxNameLen = 64;
inline constexpr unsigned kTplMaxStrSpecLen = kTplMaxNameLen + 13;
inline constexpr int kLimMin = 0;
inline constexpr int kLimMax = 1;

inline constexpr unsigned kHistLboundInf = 0x01;
inline constexpr unsigned kHistUboundInf = 0x02;

[[nodiscard]] inline Status validate_limit(int lim) noexcept {
    if (lim != kLimMin && lim != kLimMax) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_name_len(unsigned len) noexcept {
    if (len == 0 || len > kTplMaxNameLen) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::stats
