module;

export module pbsd.userland.sh.arith_yacc;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/sh/arith_yacc.c
export namespace pbsd::userland::bin::sh::arith_yacc {

[[nodiscard]] inline bool arith_yacc_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::sh::arith_yacc
