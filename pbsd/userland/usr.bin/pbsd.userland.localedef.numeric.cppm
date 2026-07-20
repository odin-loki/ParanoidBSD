module;

export module pbsd.userland.localedef.numeric;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/localedef/numeric.c
export namespace pbsd::userland::usr_bin::localedef::numeric {

[[nodiscard]] inline bool numeric_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::localedef::numeric
