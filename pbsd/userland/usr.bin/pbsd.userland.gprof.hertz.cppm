module;

export module pbsd.userland.gprof.hertz;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/gprof/hertz.c
export namespace pbsd::userland::usr_bin::gprof::hertz {

[[nodiscard]] inline bool hertz_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::gprof::hertz
