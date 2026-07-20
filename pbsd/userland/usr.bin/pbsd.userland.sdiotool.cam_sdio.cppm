module;

export module pbsd.userland.sdiotool.cam_sdio;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/sdiotool/cam_sdio.c
export namespace pbsd::userland::usr_bin::sdiotool::cam_sdio {

[[nodiscard]] inline bool cam_sdio_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::sdiotool::cam_sdio
