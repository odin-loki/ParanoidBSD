module;

export module pbsd.userland.hexdump.odsyntax;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/hexdump/odsyntax.c
export namespace pbsd::userland::usr_bin::hexdump::odsyntax {

[[nodiscard]] inline bool odsyntax_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::hexdump::odsyntax
