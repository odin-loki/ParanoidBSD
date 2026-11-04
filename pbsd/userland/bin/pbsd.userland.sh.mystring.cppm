module;

export module pbsd.userland.sh.mystring;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/sh/mystring.c
export namespace pbsd::userland::bin::sh::mystring {

[[nodiscard]] inline bool mystring_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::sh::mystring
