export module pbsd.port.wave2.hbsd.src.lib.libc.stdio.fsetpos;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdio/fsetpos.c
// void fsetpos_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdio/fsetpos.c wave=wave2 loc=45
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::fsetpos {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::fsetpos
