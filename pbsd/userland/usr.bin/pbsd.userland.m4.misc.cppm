module;

export module pbsd.userland.m4.misc;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/m4/misc.c
export namespace pbsd::userland::usr_bin::m4::misc {

[[nodiscard]] inline bool misc_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::m4::misc
