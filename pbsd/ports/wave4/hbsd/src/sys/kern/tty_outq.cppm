export module pbsd.port.wave4.hbsd.src.sys.kern.tty_outq;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/tty_outq.c
// void tty_outq_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/tty_outq.c wave=wave4 loc=344
export namespace pbsd::port::wave4::hbsd::src::sys::kern::tty_outq {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::tty_outq
