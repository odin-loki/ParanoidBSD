export module pbsd.port.wave5.hbsd.src.sys.dev.netmap.netmap_kloop;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/netmap/netmap_kloop.c
// void netmap_kloop_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/netmap/netmap_kloop.c wave=wave5 loc=1189
export namespace pbsd::port::wave5::hbsd::src::sys::dev::netmap::netmap_kloop {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::netmap::netmap_kloop
