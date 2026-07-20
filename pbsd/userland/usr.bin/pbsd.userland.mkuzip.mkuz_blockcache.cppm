module;

export module pbsd.userland.mkuzip.mkuz_blockcache;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mkuzip/mkuz_blockcache.c
export namespace pbsd::userland::usr_bin::mkuzip::mkuz_blockcache {

[[nodiscard]] inline bool mkuz_blockcache_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mkuzip::mkuz_blockcache
