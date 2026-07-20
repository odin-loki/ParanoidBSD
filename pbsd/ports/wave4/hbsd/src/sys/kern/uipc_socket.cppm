export module pbsd.port.wave4.hbsd.src.sys.kern.uipc_socket;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/uipc_socket.c
// void uipc_socket_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/uipc_socket.c wave=wave4 loc=5116
export namespace pbsd::port::wave4::hbsd::src::sys::kern::uipc_socket {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::uipc_socket
