module;

export module pbsd.userland.mkuzip.mkuz_lzma;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mkuzip/mkuz_lzma.c
export namespace pbsd::userland::usr_bin::mkuzip::mkuz_lzma {

[[nodiscard]] inline bool mkuz_lzma_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mkuzip::mkuz_lzma
