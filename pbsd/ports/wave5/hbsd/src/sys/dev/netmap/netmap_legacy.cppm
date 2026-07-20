export module pbsd.port.wave5.hbsd.src.sys.dev.netmap.netmap_legacy;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/netmap/netmap_legacy.c
// void netmap_legacy_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/netmap/netmap_legacy.c wave=wave5 loc=438
export namespace pbsd::port::wave5::hbsd::src::sys::dev::netmap::netmap_legacy {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::netmap::netmap_legacy
