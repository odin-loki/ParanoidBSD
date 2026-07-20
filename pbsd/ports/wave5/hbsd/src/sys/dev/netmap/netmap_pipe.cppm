export module pbsd.port.wave5.hbsd.src.sys.dev.netmap.netmap_pipe;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/netmap/netmap_pipe.c
// void netmap_pipe_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/netmap/netmap_pipe.c wave=wave5 loc=863
export namespace pbsd::port::wave5::hbsd::src::sys::dev::netmap::netmap_pipe {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::netmap::netmap_pipe
