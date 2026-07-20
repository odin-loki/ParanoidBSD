export module pbsd.port.wave2.hbsd.src.lib.libc.stdio.feof;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdio/feof.c
// void feof_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdio/feof.c wave=wave2 loc=59
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::feof {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::feof
