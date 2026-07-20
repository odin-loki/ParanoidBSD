module;

export module pbsd.userland.tail.misc;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tail/misc.c
export namespace pbsd::userland::usr_bin::tail::misc {

[[nodiscard]] inline bool misc_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::tail::misc
