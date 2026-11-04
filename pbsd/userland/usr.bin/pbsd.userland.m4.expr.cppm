module;

export module pbsd.userland.m4.expr;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/m4/expr.c
export namespace pbsd::userland::usr_bin::m4::expr {

[[nodiscard]] inline bool expr_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::m4::expr
