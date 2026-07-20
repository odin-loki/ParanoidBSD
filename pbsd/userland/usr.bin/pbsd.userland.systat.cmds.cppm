module;

export module pbsd.userland.systat.cmds;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/systat/cmds.c
export namespace pbsd::userland::usr_bin::systat::cmds {

[[nodiscard]] inline bool cmds_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::systat::cmds
