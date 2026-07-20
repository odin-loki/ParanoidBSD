export module pbsd.port.wave5.hbsd.src.sys.dev.pty.pty;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/pty/pty.c
// void pty_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/pty/pty.c wave=wave5 loc=163
export namespace pbsd::port::wave5::hbsd::src::sys::dev::pty::pty {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::pty::pty
