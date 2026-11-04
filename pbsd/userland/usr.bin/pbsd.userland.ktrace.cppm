module;

export module pbsd.userland.ktrace;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/ktrace/ktrace.c
export namespace pbsd::userland::usr_bin::ktrace {

[[nodiscard]] inline bool ktrace_clear(char flag) noexcept { return flag == 'C'; }

} // namespace pbsd::userland::usr_bin::ktrace
