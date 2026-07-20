export module pbsd.port.wave2.hbsd.src.lib.libc.stdlib.strtonum;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdlib/strtonum.c
// void strtonum_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdlib/strtonum.c wave=wave2 loc=65
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdlib::strtonum {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdlib::strtonum
