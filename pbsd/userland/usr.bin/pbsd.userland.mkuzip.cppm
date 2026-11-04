module;

export module pbsd.userland.mkuzip;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mkuzip/mkuzip.c
export namespace pbsd::userland::usr_bin::mkuzip {

[[nodiscard]] inline bool mkuzip_level(char flag) noexcept { return flag == 'l'; }

} // namespace pbsd::userland::usr_bin::mkuzip
