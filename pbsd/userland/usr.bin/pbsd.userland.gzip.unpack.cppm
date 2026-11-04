module;

export module pbsd.userland.gzip.unpack;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/gzip/unpack.c
export namespace pbsd::userland::usr_bin::gzip::unpack {

[[nodiscard]] inline bool unpack_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::gzip::unpack
