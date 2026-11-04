module;

export module pbsd.userland.ar.read;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/ar/read.c
export namespace pbsd::userland::usr_bin::ar::read {

[[nodiscard]] inline bool read_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::ar::read
