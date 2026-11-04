module;

export module pbsd.userland.lex.initskel;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/lex/initskel.c
export namespace pbsd::userland::usr_bin::lex::initskel {

[[nodiscard]] inline bool initskel_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::lex::initskel
