export module pbsd.port.wave2.hbsd.src.lib.libc.stdlib.atof;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdlib/atof.c
// void atof_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdlib/atof.c wave=wave2 loc=50
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdlib::atof {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdlib::atof
