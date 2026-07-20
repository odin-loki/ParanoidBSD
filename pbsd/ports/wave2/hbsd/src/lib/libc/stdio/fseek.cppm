export module pbsd.port.wave2.hbsd.src.lib.libc.stdio.fseek;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdio/fseek.c
// void fseek_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdio/fseek.c wave=wave2 loc=297
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::fseek {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::fseek
