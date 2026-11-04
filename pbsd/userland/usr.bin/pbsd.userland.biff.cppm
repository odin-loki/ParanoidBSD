module;

#include <cstdint>

export module pbsd.userland.biff;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/biff/biff.c — tty mail-notification mode bits.
export namespace pbsd::userland::usr_bin::biff {

inline constexpr unsigned kModeNotify = 0x0040U;  /* S_IXUSR */
inline constexpr unsigned kModeBell = 0x0020U;    /* S_IXGRP */

enum class Mode : char { Notify = 'y', Bell = 'b', Off = 'n' };

[[nodiscard]] inline char status_char(unsigned mode) noexcept {
    if (mode & kModeNotify) {
        return 'y';
    }
    if (mode & kModeBell) {
        return 'b';
    }
    return 'n';
}

[[nodiscard]] inline Result<Mode> parse_mode_arg(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0' || arg[1] != '\0') {
        return result_err<Mode>(Status::Invalid);
    }
    switch (arg[0]) {
    case 'y':
        return result_ok(Mode::Notify);
    case 'b':
        return result_ok(Mode::Bell);
    case 'n':
        return result_ok(Mode::Off);
    default:
        return result_err<Mode>(Status::Invalid);
    }
}

[[nodiscard]] inline unsigned apply_mode(unsigned current, Mode mode) noexcept {
    current &= ~(kModeNotify | kModeBell);
    switch (mode) {
    case Mode::Notify:
        return current | kModeNotify;
    case Mode::Bell:
        return current | kModeBell;
    case Mode::Off:
        return current;
    }
    return current;
}

} // namespace pbsd::userland::usr_bin::biff
