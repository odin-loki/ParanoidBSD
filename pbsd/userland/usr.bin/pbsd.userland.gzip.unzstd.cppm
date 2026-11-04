module;

export module pbsd.userland.gzip.unzstd;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/gzip/unzstd.c
export namespace pbsd::userland::usr_bin::gzip::unzstd {

[[nodiscard]] inline bool unzstd_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::gzip::unzstd
