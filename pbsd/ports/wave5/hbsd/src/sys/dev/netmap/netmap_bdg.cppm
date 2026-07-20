export module pbsd.port.wave5.hbsd.src.sys.dev.netmap.netmap_bdg;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/netmap/netmap_bdg.c
// void netmap_bdg_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/netmap/netmap_bdg.c wave=wave5 loc=1844
export namespace pbsd::port::wave5::hbsd::src::sys::dev::netmap::netmap_bdg {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::netmap::netmap_bdg
