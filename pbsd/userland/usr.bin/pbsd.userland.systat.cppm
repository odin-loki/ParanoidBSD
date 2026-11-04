module;

export module pbsd.userland.systat;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/systat/cmds.c
export namespace pbsd::userland::usr_bin::systat {

[[nodiscard]] inline bool systat_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::systat
