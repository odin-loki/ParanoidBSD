module;

export module pbsd.userland.sh.memalloc;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/sh/memalloc.c
export namespace pbsd::userland::bin::sh::memalloc {

[[nodiscard]] inline bool memalloc_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::sh::memalloc
