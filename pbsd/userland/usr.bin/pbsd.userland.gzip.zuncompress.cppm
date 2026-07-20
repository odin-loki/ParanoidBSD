module;

export module pbsd.userland.gzip.zuncompress;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/gzip/zuncompress.c
export namespace pbsd::userland::usr_bin::gzip::zuncompress {

[[nodiscard]] inline bool zuncompress_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::gzip::zuncompress
