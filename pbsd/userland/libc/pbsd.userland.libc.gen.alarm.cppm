module;

export module pbsd.userland.libc.gen.alarm;

export import pbsd.core;

/// alarm from hbsd/src/lib/libc/gen/alarm.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline unsigned alarm_seconds(unsigned seconds, unsigned pending) noexcept {
    const unsigned prev = pending;
    (void)seconds;
    return prev;
}

[[nodiscard]] inline Status validate_alarm(unsigned seconds) noexcept {
    if (seconds > 100000000u) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
