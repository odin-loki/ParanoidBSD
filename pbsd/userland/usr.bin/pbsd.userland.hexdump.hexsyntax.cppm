module;

export module pbsd.userland.hexdump.hexsyntax;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/hexdump/hexsyntax.c
export namespace pbsd::userland::usr_bin::hexdump::hexsyntax {

[[nodiscard]] inline bool hexsyntax_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::hexdump::hexsyntax
