export module pbsd.port.wave5.hbsd.src.sys.dev.netmap.if_ptnet;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/netmap/if_ptnet.c
// void if_ptnet_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/netmap/if_ptnet.c wave=wave5 loc=1979
export namespace pbsd::port::wave5::hbsd::src::sys::dev::netmap::if_ptnet {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::netmap::if_ptnet
