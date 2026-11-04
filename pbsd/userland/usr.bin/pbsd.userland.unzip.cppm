module;

export module pbsd.userland.unzip;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/unzip/unzip.c
export namespace pbsd::userland::usr_bin::unzip {

[[nodiscard]] inline bool unzip_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::unzip
