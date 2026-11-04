module;

export module pbsd.userland.systat.iostat;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/systat/iostat.c
export namespace pbsd::userland::usr_bin::systat::iostat {

[[nodiscard]] inline bool iostat_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::systat::iostat
