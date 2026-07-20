export module pbsd.port.wave2.hbsd.src.lib.libc.stdlib.atexit;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdlib/atexit.c
// void atexit_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdlib/atexit.c wave=wave2 loc=255
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdlib::atexit {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdlib::atexit
