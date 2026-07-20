module;

export module pbsd.userland.lex.initparse;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/lex/initparse.c
export namespace pbsd::userland::usr_bin::lex::initparse {

[[nodiscard]] inline bool initparse_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::lex::initparse
