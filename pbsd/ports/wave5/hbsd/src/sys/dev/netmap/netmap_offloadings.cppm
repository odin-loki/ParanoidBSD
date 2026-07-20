export module pbsd.port.wave5.hbsd.src.sys.dev.netmap.netmap_offloadings;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/netmap/netmap_offloadings.c
// void netmap_offloadings_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/netmap/netmap_offloadings.c wave=wave5 loc=491
export namespace pbsd::port::wave5::hbsd::src::sys::dev::netmap::netmap_offloadings {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::netmap::netmap_offloadings
