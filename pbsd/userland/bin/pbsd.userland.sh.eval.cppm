module;

export module pbsd.userland.sh.eval;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/sh/eval.c
export namespace pbsd::userland::bin::sh::eval {

[[nodiscard]] inline bool eval_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::sh::eval
