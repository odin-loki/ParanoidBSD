export module pbsd.port.wave2.hbsd.src.lib.libc.stdio.fwprintf;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdio/fwprintf.c
// void fwprintf_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdio/fwprintf.c wave=wave2 loc=62
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::fwprintf {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::fwprintf
