module;

export module pbsd.userland.xolint;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/xolint/xolint.c
export namespace pbsd::userland::usr_bin::xolint {

[[nodiscard]] inline bool xolint_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::xolint
