module;

export module pbsd.userland.sh.mknodes;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/sh/mknodes.c
export namespace pbsd::userland::bin::sh::mknodes {

[[nodiscard]] inline bool mknodes_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::sh::mknodes
