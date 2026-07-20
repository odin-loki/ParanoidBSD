module;

export module pbsd.userland.dd.args;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/dd/args.c
export namespace pbsd::userland::bin::dd::args {

[[nodiscard]] inline bool args_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::dd::args
