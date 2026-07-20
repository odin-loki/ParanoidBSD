export module pbsd.port.wave5.hbsd.src.sys.dev.netmap.netmap_generic;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/netmap/netmap_generic.c
// void netmap_generic_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/netmap/netmap_generic.c wave=wave5 loc=1185
export namespace pbsd::port::wave5::hbsd::src::sys::dev::netmap::netmap_generic {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::netmap::netmap_generic
