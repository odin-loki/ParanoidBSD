module;

export module pbsd.userland.mkuzip.mkuz_blk;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mkuzip/mkuz_blk.c
export namespace pbsd::userland::usr_bin::mkuzip::mkuz_blk {

[[nodiscard]] inline bool mkuz_blk_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mkuzip::mkuz_blk
