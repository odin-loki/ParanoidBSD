export module pbsd.port.wave2.hbsd.src.lib.libc.stdio.fgetpos;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdio/fgetpos.c
// void fgetpos_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdio/fgetpos.c wave=wave2 loc=47
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::fgetpos {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::fgetpos
