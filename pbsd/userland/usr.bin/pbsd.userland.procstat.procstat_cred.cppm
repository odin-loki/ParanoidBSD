module;

export module pbsd.userland.procstat.procstat_cred;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/procstat/procstat_cred.c
export namespace pbsd::userland::usr_bin::procstat::procstat_cred {

[[nodiscard]] inline bool procstat_cred_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::procstat::procstat_cred
