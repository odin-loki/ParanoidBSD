module;

export module pbsd.userland.mkimg.raw;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mkimg/raw.c
export namespace pbsd::userland::usr_bin::mkimg::raw {

[[nodiscard]] inline bool raw_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mkimg::raw
