export module pbsd.port.wave2.hbsd.src.lib.libc.stdio.getc;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdio/getc.c
// void getc_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdio/getc.c wave=wave2 loc=61
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::getc {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::getc
