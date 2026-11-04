module;

export module pbsd.userland.mkimg.vmdk;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mkimg/vmdk.c
export namespace pbsd::userland::usr_bin::mkimg::vmdk {

[[nodiscard]] inline bool vmdk_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mkimg::vmdk
