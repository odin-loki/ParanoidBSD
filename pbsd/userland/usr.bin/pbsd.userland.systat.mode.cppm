module;

export module pbsd.userland.systat.mode;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/systat/mode.c
export namespace pbsd::userland::usr_bin::systat::mode {

[[nodiscard]] inline bool mode_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::systat::mode
