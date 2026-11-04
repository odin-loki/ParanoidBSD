module;
#include <climits>

export module pbsd.userland.libc.gen.sleep;

export import pbsd.core;

/// sleep from hbsd/src/lib/libc/gen/sleep.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline unsigned sleep_chunk(unsigned seconds) noexcept {
    if (seconds > static_cast<unsigned>(INT_MAX)) {
        return static_cast<unsigned>(INT_MAX);
    }
    return seconds;
}

[[nodiscard]] inline unsigned sleep_remaining(unsigned requested,
                                              unsigned slept) noexcept {
    if (slept >= requested) {
        return 0;
    }
    return requested - slept;
}

[[nodiscard]] inline unsigned sleep_overflow_split(unsigned seconds) noexcept {
    if (seconds <= static_cast<unsigned>(INT_MAX)) {
        return 0;
    }
    return seconds - static_cast<unsigned>(INT_MAX);
}

} // namespace pbsd::userland::libc
