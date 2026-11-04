module;

export module pbsd.userland.localedef.scanner;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/localedef/scanner.c
export namespace pbsd::userland::usr_bin::localedef::scanner {

[[nodiscard]] inline bool scanner_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::localedef::scanner
