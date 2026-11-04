module;

export module pbsd.userland.localedef.wide;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/localedef/wide.c
export namespace pbsd::userland::usr_bin::localedef::wide {

[[nodiscard]] inline bool wide_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::localedef::wide
