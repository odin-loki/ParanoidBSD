module;

export module pbsd.userland.zstd;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/zstd/zstd.c
export namespace pbsd::userland::usr_bin::zstd {

[[nodiscard]] inline bool zstd_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::zstd
