module;

export module pbsd.userland.dd.misc;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/dd/misc.c
export namespace pbsd::userland::bin::dd::misc {

[[nodiscard]] inline bool misc_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::dd::misc
