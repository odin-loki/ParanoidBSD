module;

export module pbsd.userland.libc.gen.syslog;

export import pbsd.core;

/// syslog from hbsd/src/lib/libc/gen/syslog.c
export namespace pbsd::userland::libc {

enum class SyslogPriority : int { Emerg = 0, Alert, Crit, Err, Warning, Notice, Info, Debug };

[[nodiscard]] inline int syslog_priority_clamp(int pri) noexcept {
    if (pri < 0) {
        return 0;
    }
    if (pri > 7) {
        return 7;
    }
    return pri;
}

} // namespace pbsd::userland::libc
