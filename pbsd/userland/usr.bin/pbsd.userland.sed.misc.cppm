module;

export module pbsd.userland.sed.misc;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/sed/misc.c
export namespace pbsd::userland::usr_bin::sed::misc {

[[nodiscard]] inline bool misc_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::sed::misc
