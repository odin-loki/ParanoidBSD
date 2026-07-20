module;

export module pbsd.userland.systat.pigs;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/systat/pigs.c
export namespace pbsd::userland::usr_bin::systat::pigs {

[[nodiscard]] inline bool pigs_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::systat::pigs
