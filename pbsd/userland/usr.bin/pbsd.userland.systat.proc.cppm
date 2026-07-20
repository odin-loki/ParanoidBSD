module;

export module pbsd.userland.systat.proc;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/systat/proc.c
export namespace pbsd::userland::usr_bin::systat::proc {

[[nodiscard]] inline bool proc_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::systat::proc
