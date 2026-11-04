module;

export module pbsd.userland.mkcsmapper_static;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mkcsmapper_static/mkcsmapper_static.c
export namespace pbsd::userland::usr_bin::mkcsmapper_static {

[[nodiscard]] inline bool mkcsmapper_static_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::mkcsmapper_static
