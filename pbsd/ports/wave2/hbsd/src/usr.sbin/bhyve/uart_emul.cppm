export module pbsd.port.wave2.hbsd.src.usr_sbin.bhyve.uart_emul;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/bhyve/uart_emul.c
// void uart_emul_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/bhyve/uart_emul.c wave=wave2 loc=485
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::uart_emul {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::uart_emul
