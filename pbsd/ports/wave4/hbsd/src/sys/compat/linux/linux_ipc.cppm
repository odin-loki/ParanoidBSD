export module pbsd.port.wave4.hbsd.src.sys.compat.linux.linux_ipc;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/compat/linux/linux_ipc.c
// void linux_ipc_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/compat/linux/linux_ipc.c wave=wave4 loc=915
export namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_ipc {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_ipc
