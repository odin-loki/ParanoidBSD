module;

export module pbsd.userland.sort.file;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/sort/file.c
export namespace pbsd::userland::usr_bin::sort::file {

[[nodiscard]] inline bool file_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::sort::file
