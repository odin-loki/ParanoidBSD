export module pbsd.port.wave5.hbsd.src.sys.dev.netmap.netmap_freebsd;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/netmap/netmap_freebsd.c
// void netmap_freebsd_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/netmap/netmap_freebsd.c wave=wave5 loc=1612
export namespace pbsd::port::wave5::hbsd::src::sys::dev::netmap::netmap_freebsd {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::netmap::netmap_freebsd
