module;

export module pbsd.userland.libc.gen.usleep;

export import pbsd.core;

/// usleep from hbsd/src/lib/libc/gen/usleep.c
export namespace pbsd::userland::libc {

struct Timespec {
    long sec{};
    long nsec{};
};

[[nodiscard]] inline Timespec usec_to_timespec(unsigned usec) noexcept {
    Timespec ts{};
    ts.nsec = static_cast<long>((usec % 1000000u) * 1000u);
    ts.sec = static_cast<long>(usec / 1000000u);
    return ts;
}

[[nodiscard]] inline unsigned timespec_to_usec(Timespec ts) noexcept {
    if (ts.sec < 0 || ts.nsec < 0) {
        return 0;
    }
    return static_cast<unsigned>(ts.sec) * 1000000u
         + static_cast<unsigned>(ts.nsec / 1000);
}

} // namespace pbsd::userland::libc
