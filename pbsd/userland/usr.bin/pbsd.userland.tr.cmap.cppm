module;

export module pbsd.userland.tr.cmap;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tr/cmap.c
export namespace pbsd::userland::usr_bin::tr::cmap {

[[nodiscard]] inline bool cmap_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::tr::cmap
