export module pbsd.port.wave9.hbsd.src.contrib.tcsh.mi_termios;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tcsh/mi.termios.c
// void mi.termios_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tcsh/mi.termios.c wave=wave9 loc=391
export namespace pbsd::port::wave9::hbsd::src::contrib::tcsh::mi_termios {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tcsh::mi_termios
