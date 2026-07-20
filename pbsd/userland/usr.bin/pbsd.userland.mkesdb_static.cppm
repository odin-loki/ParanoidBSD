module;

export module pbsd.userland.mkesdb_static;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mkesdb_static/mkesdb_static.c
export namespace pbsd::userland::usr_bin::mkesdb_static {

[[nodiscard]] inline bool mkesdb_static_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::mkesdb_static
