export module pbsd.port.wave4.hbsd.src.sys.kern.subr_terminal;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/subr_terminal.c
// void subr_terminal_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/subr_terminal.c wave=wave4 loc=709
export namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_terminal {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_terminal
