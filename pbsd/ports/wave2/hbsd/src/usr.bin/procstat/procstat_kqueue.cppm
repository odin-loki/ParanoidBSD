export module pbsd.port.wave2.hbsd.src.usr_bin.procstat.procstat_kqueue;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/procstat/procstat_kqueue.c
// void procstat_kqueue_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/procstat/procstat_kqueue.c wave=wave2 loc=319
export namespace pbsd::port::wave2::hbsd::src::usr_bin::procstat::procstat_kqueue {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::procstat::procstat_kqueue
