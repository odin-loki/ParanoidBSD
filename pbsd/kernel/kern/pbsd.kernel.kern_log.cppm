module;

export module pbsd.kernel.kern_log;

export import pbsd.core;

/// Freestanding port of `kern/subr_log.c` — log(9) priorities.
export namespace pbsd::kernel::kern_log {

inline constexpr int kEmerg = 0;
inline constexpr int kAlert = 1;
inline constexpr int kCrit = 2;
inline constexpr int kErr = 3;
inline constexpr int kWarning = 4;
inline constexpr int kNotice = 5;
inline constexpr int kInfo = 6;
inline constexpr int kDebug = 7;

[[nodiscard]] inline Status validate_pri(int pri) noexcept {
    if (pri < kEmerg || pri > kDebug) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* pri_name(int pri) noexcept {
    switch (pri) {
    case kEmerg: return "emerg";
    case kAlert: return "alert";
    case kCrit: return "crit";
    case kErr: return "err";
    case kWarning: return "warning";
    case kNotice: return "notice";
    case kInfo: return "info";
    case kDebug: return "debug";
    default: return nullptr;
    }
}

} // namespace pbsd::kernel::kern_log
