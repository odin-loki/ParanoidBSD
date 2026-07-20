export module pbsd.port.wave2.hbsd.src.lib.libc.gen.termios;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/gen/termios.c
// void termios_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/gen/termios.c wave=wave2 loc=287
export namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::termios {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::termios
