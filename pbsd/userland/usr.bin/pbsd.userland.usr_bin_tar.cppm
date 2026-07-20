module;

export module pbsd.userland.usr_bin_tar;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tar/tar.c
export namespace pbsd::userland::usr_bin::usr_bin_tar {

[[nodiscard]] inline bool usr_bin_tar_flag(char c) noexcept { return c == 't'; }

} // namespace pbsd::userland::usr_bin::usr_bin_tar
