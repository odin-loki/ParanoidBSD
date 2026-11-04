module;

export module pbsd.userland.localedef.monetary;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/localedef/monetary.c
export namespace pbsd::userland::usr_bin::localedef::monetary {

[[nodiscard]] inline bool monetary_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::localedef::monetary
