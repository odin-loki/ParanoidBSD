module;

export module pbsd.userland.localedef.collate;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/localedef/collate.c
export namespace pbsd::userland::usr_bin::localedef::collate {

[[nodiscard]] inline bool collate_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::localedef::collate
