module;

export module pbsd.userland.mkimg.apm;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mkimg/apm.c
export namespace pbsd::userland::usr_bin::mkimg::apm {

[[nodiscard]] inline bool apm_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mkimg::apm
