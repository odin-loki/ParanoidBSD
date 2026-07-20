module;

export module pbsd.userland.mkimg.format;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mkimg/format.c
export namespace pbsd::userland::usr_bin::mkimg::format {

[[nodiscard]] inline bool format_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mkimg::format
