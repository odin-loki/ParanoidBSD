export module pbsd.port.wave4.hbsd.src.sys.kern.uipc_usrreq;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/uipc_usrreq.c
// void uipc_usrreq_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/uipc_usrreq.c wave=wave4 loc=4643
export namespace pbsd::port::wave4::hbsd::src::sys::kern::uipc_usrreq {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::uipc_usrreq
