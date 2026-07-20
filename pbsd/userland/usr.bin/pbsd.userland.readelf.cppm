module;

export module pbsd.userland.readelf;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/readelf/readelf.c
export namespace pbsd::userland::usr_bin::readelf {

[[nodiscard]] inline bool readelf_wide(char flag) noexcept { return flag == 'W'; }

} // namespace pbsd::userland::usr_bin::readelf
