module;

export module pbsd.userland.ktrace.subr;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/ktrace/subr.c
export namespace pbsd::userland::usr_bin::ktrace::subr {

[[nodiscard]] inline bool subr_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::ktrace::subr
