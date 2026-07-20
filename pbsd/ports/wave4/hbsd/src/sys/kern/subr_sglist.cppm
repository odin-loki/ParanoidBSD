export module pbsd.port.wave4.hbsd.src.sys.kern.subr_sglist;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/subr_sglist.c
// void subr_sglist_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/subr_sglist.c wave=wave4 loc=983
export namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_sglist {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_sglist
