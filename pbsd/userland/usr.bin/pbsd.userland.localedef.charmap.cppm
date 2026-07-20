module;

export module pbsd.userland.localedef.charmap;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/localedef/charmap.c
export namespace pbsd::userland::usr_bin::localedef::charmap {

[[nodiscard]] inline bool charmap_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::localedef::charmap
