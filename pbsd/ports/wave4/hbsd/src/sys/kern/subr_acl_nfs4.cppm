export module pbsd.port.wave4.hbsd.src.sys.kern.subr_acl_nfs4;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/subr_acl_nfs4.c
// void subr_acl_nfs4_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/subr_acl_nfs4.c wave=wave4 loc=1411
export namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_acl_nfs4 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_acl_nfs4
