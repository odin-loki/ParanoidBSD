module;

export module pbsd.userland.setfacl.merge;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/setfacl/merge.c
export namespace pbsd::userland::bin::setfacl::merge {

[[nodiscard]] inline bool merge_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::setfacl::merge
