module;

export module pbsd.userland.cap_mkdb;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/cap_mkdb/cap_mkdb.c
export namespace pbsd::userland::usr_bin::cap_mkdb {

[[nodiscard]] inline bool cap_mkdb_verbose(char flag) noexcept { return flag == 'v'; }

} // namespace pbsd::userland::usr_bin::cap_mkdb
