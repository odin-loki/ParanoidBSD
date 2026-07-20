export module pbsd.port.wave4.hbsd.src.sys.kern.uipc_mqueue;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/uipc_mqueue.c
// void uipc_mqueue_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/uipc_mqueue.c wave=wave4 loc=2953
export namespace pbsd::port::wave4::hbsd::src::sys::kern::uipc_mqueue {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::uipc_mqueue
