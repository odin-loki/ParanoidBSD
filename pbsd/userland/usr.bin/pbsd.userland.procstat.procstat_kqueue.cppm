module;

export module pbsd.userland.procstat.procstat_kqueue;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/procstat/procstat_kqueue.c
export namespace pbsd::userland::usr_bin::procstat::procstat_kqueue {

[[nodiscard]] inline bool procstat_kqueue_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::procstat::procstat_kqueue
