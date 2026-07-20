export module pbsd.port.wave2.hbsd.src.lib.libc.stdio.vfprintf;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdio/vfprintf.c
// void vfprintf_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdio/vfprintf.c wave=wave2 loc=1089
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::vfprintf {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::vfprintf
