export module pbsd.port.wave4.hbsd.src.sys.kern.subr_kdb;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/subr_kdb.c
// void subr_kdb_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/subr_kdb.c wave=wave4 loc=814
export namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_kdb {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_kdb
