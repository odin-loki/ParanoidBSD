module;

export module pbsd.userland.systat.netcmds;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/systat/netcmds.c
export namespace pbsd::userland::usr_bin::systat::netcmds {

[[nodiscard]] inline bool netcmds_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::systat::netcmds
