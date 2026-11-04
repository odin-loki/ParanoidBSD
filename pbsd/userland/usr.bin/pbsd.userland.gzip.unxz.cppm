module;

export module pbsd.userland.gzip.unxz;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/gzip/unxz.c
export namespace pbsd::userland::usr_bin::gzip::unxz {

[[nodiscard]] inline bool unxz_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::gzip::unxz
