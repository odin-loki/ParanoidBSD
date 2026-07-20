export module pbsd.port.wave2.hbsd.src.lib.libc.stdlib.quick_exit;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdlib/quick_exit.c
// void quick_exit_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdlib/quick_exit.c wave=wave2 loc=77
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdlib::quick_exit {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdlib::quick_exit
