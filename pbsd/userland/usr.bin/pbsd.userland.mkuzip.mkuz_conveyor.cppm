module;

export module pbsd.userland.mkuzip.mkuz_conveyor;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mkuzip/mkuz_conveyor.c
export namespace pbsd::userland::usr_bin::mkuzip::mkuz_conveyor {

[[nodiscard]] inline bool mkuz_conveyor_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mkuzip::mkuz_conveyor
