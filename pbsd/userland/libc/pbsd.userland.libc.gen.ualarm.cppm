module;

export module pbsd.userland.libc.gen.ualarm;

export import pbsd.core;

/// ualarm from hbsd/src/lib/libc/gen/ualarm.c
export namespace pbsd::userland::libc {

inline constexpr unsigned kUsecPerSec = 1000000u;

[[nodiscard]] inline unsigned usec_remainder(unsigned usec, unsigned elapsed) noexcept {
    if (elapsed >= usec) {
        return 0;
    }
    return usec - elapsed;
}

[[nodiscard]] inline unsigned usec_interval(unsigned usec, unsigned interval) noexcept {
    if (interval == 0) {
        return usec;
    }
    return interval;
}

} // namespace pbsd::userland::libc
