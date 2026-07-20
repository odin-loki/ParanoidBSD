module;

export module pbsd.userland.localedef.ctype;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/localedef/ctype.c
export namespace pbsd::userland::usr_bin::localedef::ctype {

[[nodiscard]] inline bool ctype_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::localedef::ctype
