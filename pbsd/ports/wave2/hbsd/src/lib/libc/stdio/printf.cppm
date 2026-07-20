export module pbsd.port.wave2.hbsd.src.lib.libc.stdio.printf;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdio/printf.c
// void printf_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdio/printf.c wave=wave2 loc=65
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::printf {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::printf
