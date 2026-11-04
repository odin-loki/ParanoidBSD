module;

export module pbsd.userland.mkuzip.mkuz_zlib;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mkuzip/mkuz_zlib.c
export namespace pbsd::userland::usr_bin::mkuzip::mkuz_zlib {

[[nodiscard]] inline bool mkuz_zlib_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mkuzip::mkuz_zlib
