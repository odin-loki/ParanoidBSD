module;

export module pbsd.userland.systat.cmdtab;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/systat/cmdtab.c
export namespace pbsd::userland::usr_bin::systat::cmdtab {

[[nodiscard]] inline bool cmdtab_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::systat::cmdtab
