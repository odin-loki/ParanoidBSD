module;

export module pbsd.userland.setfacl.remove;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/setfacl/remove.c
export namespace pbsd::userland::bin::setfacl::remove {

[[nodiscard]] inline bool remove_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::setfacl::remove
