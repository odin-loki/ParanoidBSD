export module pbsd.port.wave2.hbsd.src.lib.libc.stdio.fgetln;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdio/fgetln.c
// void fgetln_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdio/fgetln.c wave=wave2 loc=160
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::fgetln {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::fgetln
