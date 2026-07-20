module;

export module pbsd.userland.setfacl.mask;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/setfacl/mask.c
export namespace pbsd::userland::bin::setfacl::mask {

[[nodiscard]] inline bool mask_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::setfacl::mask
