module;

export module pbsd.userland.sdiotool.linux_sdio_compat;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/sdiotool/linux_sdio_compat.c
export namespace pbsd::userland::usr_bin::sdiotool::linux_sdio_compat {

[[nodiscard]] inline bool linux_sdio_compat_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::sdiotool::linux_sdio_compat
