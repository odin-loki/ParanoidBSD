module;

export module pbsd.kernel.subr_prf;

import pbsd.core;

/// Freestanding port of `subr_prf.c` — printf severity levels.
export namespace pbsd::kernel::subr_prf {

enum class LogLevel : unsigned char {
    Emergency = 0,
    Alert = 1,
    Critical = 2,
    Error = 3,
    Warning = 4,
    Notice = 5,
    Info = 6,
    Debug = 7,
};

[[nodiscard]] inline bool level_enabled(LogLevel cur, LogLevel msg) noexcept {
    return static_cast<unsigned>(msg) <= static_cast<unsigned>(cur);
}

[[nodiscard]] inline char const* level_name(LogLevel lvl) noexcept {
    switch (lvl) {
    case LogLevel::Emergency: return "EMERG";
    case LogLevel::Alert: return "ALERT";
    case LogLevel::Critical: return "CRIT";
    case LogLevel::Error: return "ERR";
    case LogLevel::Warning: return "WARN";
    case LogLevel::Notice: return "NOTICE";
    case LogLevel::Info: return "INFO";
    case LogLevel::Debug: return "DEBUG";
    default: return "?";
    }
}

[[nodiscard]] inline Status validate_level(unsigned lvl) noexcept {
    if (lvl > static_cast<unsigned>(LogLevel::Debug)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::subr_prf
