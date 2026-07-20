module;

export module pbsd.userland.lzmainfo;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/contrib/xz/src/lzmainfo/lzmainfo.c
export namespace pbsd::userland::usr_bin::lzmainfo {

[[nodiscard]] inline bool lzmainfo_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::lzmainfo
