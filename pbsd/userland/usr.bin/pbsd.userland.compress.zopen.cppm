module;

export module pbsd.userland.compress.zopen;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/compress/zopen.c
export namespace pbsd::userland::usr_bin::compress::zopen {

[[nodiscard]] inline bool zopen_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::compress::zopen
