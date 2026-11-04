module;

export module pbsd.userland.w.proc_compare;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/w/proc_compare.c
export namespace pbsd::userland::usr_bin::w::proc_compare {

[[nodiscard]] inline bool proc_compare_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::w::proc_compare
