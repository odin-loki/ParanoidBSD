export module pbsd.port.wave2.hbsd.src.lib.libc.stdio.ungetc;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdio/ungetc.c
// void ungetc_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdio/ungetc.c wave=wave2 loc=164
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::ungetc {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::ungetc
