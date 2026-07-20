export module pbsd.port.wave2.hbsd.src.lib.libc.stdio.stdio;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdio/stdio.c
// void stdio_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdio/stdio.c wave=wave2 loc=172
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::stdio {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::stdio
