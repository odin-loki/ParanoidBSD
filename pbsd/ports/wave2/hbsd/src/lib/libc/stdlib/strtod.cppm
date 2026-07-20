export module pbsd.port.wave2.hbsd.src.lib.libc.stdlib.strtod;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdlib/strtod.c
// void strtod_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: converted
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdlib/strtod.c wave=wave2 loc=0
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdlib::strtod {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdlib::strtod
