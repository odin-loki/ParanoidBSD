export module pbsd.port.wave2.hbsd.src.lib.libc.stdio.putchar;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdio/putchar.c
// void putchar_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdio/putchar.c wave=wave2 loc=66
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::putchar {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::putchar
