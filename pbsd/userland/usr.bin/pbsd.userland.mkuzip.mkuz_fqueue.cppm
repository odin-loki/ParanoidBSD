module;

export module pbsd.userland.mkuzip.mkuz_fqueue;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mkuzip/mkuz_fqueue.c
export namespace pbsd::userland::usr_bin::mkuzip::mkuz_fqueue {

[[nodiscard]] inline bool mkuz_fqueue_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mkuzip::mkuz_fqueue
