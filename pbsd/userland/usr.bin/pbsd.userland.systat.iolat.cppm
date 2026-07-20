module;

export module pbsd.userland.systat.iolat;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/systat/iolat.c
export namespace pbsd::userland::usr_bin::systat::iolat {

[[nodiscard]] inline bool iolat_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::systat::iolat
