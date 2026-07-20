export module pbsd.port.wave2.hbsd.src.lib.libc.stdio.clrerr;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdio/clrerr.c
// void clrerr_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdio/clrerr.c wave=wave2 loc=56
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::clrerr {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::clrerr
