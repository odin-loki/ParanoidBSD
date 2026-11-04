module;

export module pbsd.userland.setfacl.util;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/setfacl/util.c
export namespace pbsd::userland::bin::setfacl::util {

[[nodiscard]] inline bool util_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::setfacl::util
