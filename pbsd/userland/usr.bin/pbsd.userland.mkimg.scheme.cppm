module;

export module pbsd.userland.mkimg.scheme;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mkimg/scheme.c
export namespace pbsd::userland::usr_bin::mkimg::scheme {

[[nodiscard]] inline bool scheme_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mkimg::scheme
