export module pbsd.port.wave2.hbsd.src.lib.libc.stdlib.atoi;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdlib/atoi.c
// void atoi_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdlib/atoi.c wave=wave2 loc=50
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdlib::atoi {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdlib::atoi
