export module pbsd.port.wave4.hbsd.src.sys.kern.subr_kobj;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/subr_kobj.c
// void subr_kobj_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/subr_kobj.c wave=wave4 loc=351
export namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_kobj {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_kobj
