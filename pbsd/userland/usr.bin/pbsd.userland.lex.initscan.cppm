module;

export module pbsd.userland.lex.initscan;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/lex/initscan.c
export namespace pbsd::userland::usr_bin::lex::initscan {

[[nodiscard]] inline bool initscan_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::lex::initscan
