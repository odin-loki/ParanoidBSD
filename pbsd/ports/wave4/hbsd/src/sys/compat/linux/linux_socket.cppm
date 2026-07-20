export module pbsd.port.wave4.hbsd.src.sys.compat.linux.linux_socket;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/compat/linux/linux_socket.c
// void linux_socket_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/compat/linux/linux_socket.c wave=wave4 loc=2815
export namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_socket {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_socket
