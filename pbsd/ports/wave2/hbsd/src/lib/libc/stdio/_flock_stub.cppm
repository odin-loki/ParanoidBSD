export module pbsd.port.wave2.hbsd.src.lib.libc.stdio._flock_stub;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdio/_flock_stub.c
// void _flock_stub_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdio/_flock_stub.c wave=wave2 loc=138
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::_flock_stub {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::_flock_stub
