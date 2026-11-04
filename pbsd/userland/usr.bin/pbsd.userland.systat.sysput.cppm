module;

export module pbsd.userland.systat.sysput;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/systat/sysput.c
export namespace pbsd::userland::usr_bin::systat::sysput {

[[nodiscard]] inline bool sysput_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::systat::sysput
