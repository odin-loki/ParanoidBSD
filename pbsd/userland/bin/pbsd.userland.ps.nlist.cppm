module;

export module pbsd.userland.ps.nlist;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/ps/nlist.c
export namespace pbsd::userland::bin::ps::nlist {

[[nodiscard]] inline bool nlist_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::ps::nlist
