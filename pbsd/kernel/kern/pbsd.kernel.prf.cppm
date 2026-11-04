module;

export module pbsd.kernel.prf;

export import pbsd.core;
export import pbsd.kernel.subr_prf;

/// Freestanding port of `sys/kern/subr_prf.h` — printf(9) message flags.
export namespace pbsd::kernel::prf {

using subr_prf::LogLevel;

inline constexpr unsigned kMsgEmergency = 1u << 0;
inline constexpr unsigned kMsgAlert     = 1u << 1;
inline constexpr unsigned kMsgCritical  = 1u << 2;
inline constexpr unsigned kMsgError     = 1u << 3;
inline constexpr unsigned kMsgWarning   = 1u << 4;
inline constexpr unsigned kMsgNotice    = 1u << 5;
inline constexpr unsigned kMsgInfo      = 1u << 6;
inline constexpr unsigned kMsgDebug     = 1u << 7;

[[nodiscard]] inline LogLevel flag_to_level(unsigned flags) noexcept {
    if ((flags & kMsgEmergency) != 0) return LogLevel::Emergency;
    if ((flags & kMsgAlert) != 0) return LogLevel::Alert;
    if ((flags & kMsgCritical) != 0) return LogLevel::Critical;
    if ((flags & kMsgError) != 0) return LogLevel::Error;
    if ((flags & kMsgWarning) != 0) return LogLevel::Warning;
    if ((flags & kMsgNotice) != 0) return LogLevel::Notice;
    if ((flags & kMsgDebug) != 0) return LogLevel::Debug;
    return LogLevel::Info;
}

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    if (flags == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::prf
