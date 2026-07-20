export module pbsd.port.wave2.hbsd.src.lib.libc.sys.wait;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/sys/wait.c
// void wait_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/sys/wait.c wave=wave2 loc=50
export namespace pbsd::port::wave2::hbsd::src::lib::libc::sys::wait {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::sys::wait
