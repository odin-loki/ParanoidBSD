module;

export module pbsd.userland.vgrind.vfontedpr;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/vgrind/vfontedpr.c
export namespace pbsd::userland::usr_bin::vgrind::vfontedpr {

[[nodiscard]] inline bool vfontedpr_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::vgrind::vfontedpr
