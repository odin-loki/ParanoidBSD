module;

export module pbsd.userland.at.panic;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/at/panic.c
export namespace pbsd::userland::usr_bin::at::panic {

[[nodiscard]] inline bool panic_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::at::panic
