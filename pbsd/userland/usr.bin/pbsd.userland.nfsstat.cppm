module;

export module pbsd.userland.nfsstat;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/nfsstat/nfsstat.c
export namespace pbsd::userland::usr_bin::nfsstat {

[[nodiscard]] inline bool nfsstat_json(char c) noexcept { return c == 'j'; }

} // namespace pbsd::userland::usr_bin::nfsstat
