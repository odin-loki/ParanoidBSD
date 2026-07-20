export module pbsd.port.wave2.hbsd.src.lib.libc.stdlib.exit;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdlib/exit.c
// void exit_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdlib/exit.c wave=wave2 loc=93
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdlib::exit {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdlib::exit
