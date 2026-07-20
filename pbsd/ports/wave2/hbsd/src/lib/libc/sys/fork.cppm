export module pbsd.port.wave2.hbsd.src.lib.libc.sys.fork;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/sys/fork.c
// void fork_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/sys/fork.c wave=wave2 loc=41
export namespace pbsd::port::wave2::hbsd::src::lib::libc::sys::fork {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::sys::fork
