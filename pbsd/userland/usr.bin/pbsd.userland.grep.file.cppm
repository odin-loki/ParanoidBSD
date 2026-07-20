module;

export module pbsd.userland.grep.file;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/grep/file.c
export namespace pbsd::userland::usr_bin::grep::file {

[[nodiscard]] inline bool file_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::grep::file
