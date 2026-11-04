module;

export module pbsd.userland.ctlstat;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/ctlstat/ctlstat.c
export namespace pbsd::userland::usr_bin::ctlstat {

[[nodiscard]] inline bool ctlstat_json(char flag) noexcept { return flag == 'j'; }

} // namespace pbsd::userland::usr_bin::ctlstat
