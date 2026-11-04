module;

export module pbsd.userland.procstat.procstat_pwdx;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/procstat/procstat_pwdx.c
export namespace pbsd::userland::usr_bin::procstat::procstat_pwdx {

[[nodiscard]] inline bool procstat_pwdx_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::procstat::procstat_pwdx
