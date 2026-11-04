module;

export module pbsd.userland.mkimg;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mkimg/mkimg.c
export namespace pbsd::userland::usr_bin::mkimg {

[[nodiscard]] inline bool mkimg_verbose(char flag) noexcept { return flag == 'v'; }

} // namespace pbsd::userland::usr_bin::mkimg
