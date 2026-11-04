module;

export module pbsd.userland.mkuzip.mkuz_insize;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mkuzip/mkuz_insize.c
export namespace pbsd::userland::usr_bin::mkuzip::mkuz_insize {

[[nodiscard]] inline bool mkuz_insize_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mkuzip::mkuz_insize
