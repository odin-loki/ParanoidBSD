module;

export module pbsd.userland.vgrind.regexp;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/vgrind/regexp.c
export namespace pbsd::userland::usr_bin::vgrind::regexp {

[[nodiscard]] inline bool regexp_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::vgrind::regexp
