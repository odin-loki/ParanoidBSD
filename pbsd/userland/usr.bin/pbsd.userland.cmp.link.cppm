module;

export module pbsd.userland.cmp.link;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/cmp/link.c
export namespace pbsd::userland::usr_bin::cmp::link {

[[nodiscard]] inline bool link_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::cmp::link
