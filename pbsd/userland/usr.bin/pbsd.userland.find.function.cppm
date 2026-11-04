module;

export module pbsd.userland.find.function;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/find/function.c
export namespace pbsd::userland::usr_bin::find::function {

[[nodiscard]] inline bool function_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::find::function
