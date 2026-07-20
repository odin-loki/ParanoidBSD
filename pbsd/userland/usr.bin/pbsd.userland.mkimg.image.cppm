module;

export module pbsd.userland.mkimg.image;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mkimg/image.c
export namespace pbsd::userland::usr_bin::mkimg::image {

[[nodiscard]] inline bool image_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mkimg::image
