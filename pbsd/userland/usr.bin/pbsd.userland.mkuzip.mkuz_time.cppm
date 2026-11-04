module;

export module pbsd.userland.mkuzip.mkuz_time;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mkuzip/mkuz_time.c
export namespace pbsd::userland::usr_bin::mkuzip::mkuz_time {

[[nodiscard]] inline bool mkuz_time_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mkuzip::mkuz_time
