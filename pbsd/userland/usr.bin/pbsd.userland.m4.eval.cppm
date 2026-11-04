module;

export module pbsd.userland.m4.eval;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/m4/eval.c
export namespace pbsd::userland::usr_bin::m4::eval {

[[nodiscard]] inline bool eval_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::m4::eval
