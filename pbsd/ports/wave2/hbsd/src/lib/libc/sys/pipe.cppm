export module pbsd.port.wave2.hbsd.src.lib.libc.sys.pipe;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/sys/pipe.c
// void pipe_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/sys/pipe.c wave=wave2 loc=44
export namespace pbsd::port::wave2::hbsd::src::lib::libc::sys::pipe {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::sys::pipe
