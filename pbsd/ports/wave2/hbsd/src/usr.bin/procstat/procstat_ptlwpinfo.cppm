export module pbsd.port.wave2.hbsd.src.usr_bin.procstat.procstat_ptlwpinfo;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/procstat/procstat_ptlwpinfo.c
// void procstat_ptlwpinfo_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/procstat/procstat_ptlwpinfo.c wave=wave2 loc=92
export namespace pbsd::port::wave2::hbsd::src::usr_bin::procstat::procstat_ptlwpinfo {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::procstat::procstat_ptlwpinfo
