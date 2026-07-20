module;

export module pbsd.userland.systat.swap;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/systat/swap.c
export namespace pbsd::userland::usr_bin::systat::swap {

[[nodiscard]] inline bool swap_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::systat::swap
