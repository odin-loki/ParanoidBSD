module;

export module pbsd.userland.w.pr_time;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/w/pr_time.c
export namespace pbsd::userland::usr_bin::w::pr_time {

[[nodiscard]] inline bool pr_time_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::w::pr_time
