module;

export module pbsd.userland.mkuzip.mkuz_zstd;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mkuzip/mkuz_zstd.c
export namespace pbsd::userland::usr_bin::mkuzip::mkuz_zstd {

[[nodiscard]] inline bool mkuz_zstd_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mkuzip::mkuz_zstd
