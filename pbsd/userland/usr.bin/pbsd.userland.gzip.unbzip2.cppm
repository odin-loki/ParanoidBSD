module;

export module pbsd.userland.gzip.unbzip2;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/gzip/unbzip2.c
export namespace pbsd::userland::usr_bin::gzip::unbzip2 {

[[nodiscard]] inline bool unbzip2_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::gzip::unbzip2
