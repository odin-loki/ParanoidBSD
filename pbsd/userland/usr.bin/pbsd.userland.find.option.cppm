module;

export module pbsd.userland.find.option;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/find/option.c
export namespace pbsd::userland::usr_bin::find::option {

[[nodiscard]] inline bool option_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::find::option
