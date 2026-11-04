module;

export module pbsd.userland.xo;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/xo/xo.c
export namespace pbsd::userland::usr_bin::xo {

[[nodiscard]] inline bool xo_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::xo
