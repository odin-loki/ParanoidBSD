export module pbsd.port.wave5.hbsd.src.sys.dev.virtio.network.if_vtnet;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/virtio/network/if_vtnet.c
// void if_vtnet_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/virtio/network/if_vtnet.c wave=wave5 loc=4656
export namespace pbsd::port::wave5::hbsd::src::sys::dev::virtio::network::if_vtnet {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::virtio::network::if_vtnet
