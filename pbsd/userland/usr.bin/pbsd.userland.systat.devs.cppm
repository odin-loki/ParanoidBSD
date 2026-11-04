module;

export module pbsd.userland.systat.devs;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/systat/devs.c
export namespace pbsd::userland::usr_bin::systat::devs {

[[nodiscard]] inline bool devs_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::systat::devs
