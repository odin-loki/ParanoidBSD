export module pbsd.port.wave2.hbsd.src.lib.libc.stdlib.atol;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdlib/atol.c
// void atol_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdlib/atol.c wave=wave2 loc=50
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdlib::atol {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdlib::atol
