module;

export module pbsd.userland.procstat;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/procstat/procstat.c
export namespace pbsd::userland::usr_bin::procstat {

[[nodiscard]] inline bool procstat_json(char c) noexcept { return c == 'j'; }

} // namespace pbsd::userland::usr_bin::procstat
