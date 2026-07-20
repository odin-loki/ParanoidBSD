module;

export module pbsd.userland.tip.tip.vars;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tip/tip/vars.c
export namespace pbsd::userland::usr_bin::tip::tip::vars {

[[nodiscard]] inline bool tip_vars_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::tip::tip::vars
