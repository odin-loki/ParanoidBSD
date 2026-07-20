module;

export module pbsd.userland.sh.arith_yylex;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/sh/arith_yylex.c
export namespace pbsd::userland::bin::sh::arith_yylex {

[[nodiscard]] inline bool arith_yylex_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::sh::arith_yylex
