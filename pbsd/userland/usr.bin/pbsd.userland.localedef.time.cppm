module;

export module pbsd.userland.localedef.time;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/localedef/time.c
export namespace pbsd::userland::usr_bin::localedef::time {

[[nodiscard]] inline bool time_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::localedef::time
