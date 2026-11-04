module;

export module pbsd.userland.mkesdb;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mkesdb/mkesdb.c
export namespace pbsd::userland::usr_bin::mkesdb {

[[nodiscard]] inline bool mkesdb_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::mkesdb
