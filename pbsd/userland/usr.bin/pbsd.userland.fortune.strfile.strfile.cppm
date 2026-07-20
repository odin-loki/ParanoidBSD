module;

export module pbsd.userland.fortune.strfile.strfile;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/fortune/strfile/strfile.c
export namespace pbsd::userland::usr_bin::fortune::strfile::strfile {

[[nodiscard]] inline bool strfile_strfile_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::fortune::strfile::strfile
