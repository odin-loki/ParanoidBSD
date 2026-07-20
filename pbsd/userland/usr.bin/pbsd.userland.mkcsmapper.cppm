module;

export module pbsd.userland.mkcsmapper;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mkcsmapper/mkcsmapper.c
export namespace pbsd::userland::usr_bin::mkcsmapper {

[[nodiscard]] inline bool mkcsmapper_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::mkcsmapper
