module;

export module pbsd.userland.gzip.unlz;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/gzip/unlz.c
export namespace pbsd::userland::usr_bin::gzip::unlz {

[[nodiscard]] inline bool unlz_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::gzip::unlz
