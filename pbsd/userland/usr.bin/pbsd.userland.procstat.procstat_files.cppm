module;

export module pbsd.userland.procstat.procstat_files;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/procstat/procstat_files.c
export namespace pbsd::userland::usr_bin::procstat::procstat_files {

[[nodiscard]] inline bool procstat_files_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::procstat::procstat_files
