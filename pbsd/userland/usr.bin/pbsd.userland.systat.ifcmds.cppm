module;

export module pbsd.userland.systat.ifcmds;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/systat/ifcmds.c
export namespace pbsd::userland::usr_bin::systat::ifcmds {

[[nodiscard]] inline bool ifcmds_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::systat::ifcmds
