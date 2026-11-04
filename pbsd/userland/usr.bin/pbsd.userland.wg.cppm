module;

export module pbsd.userland.wg;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/wg/wg.c
export namespace pbsd::userland::usr_bin::wg {

[[nodiscard]] inline bool wg_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::wg
