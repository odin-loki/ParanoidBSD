module;

export module pbsd.userland.systat.convtbl;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/systat/convtbl.c
export namespace pbsd::userland::usr_bin::systat::convtbl {

[[nodiscard]] inline bool convtbl_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::systat::convtbl
