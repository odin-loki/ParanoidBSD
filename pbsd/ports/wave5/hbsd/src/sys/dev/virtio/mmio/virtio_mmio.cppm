export module pbsd.port.wave5.hbsd.src.sys.dev.virtio.mmio.virtio_mmio;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/virtio/mmio/virtio_mmio.c
// void virtio_mmio_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/virtio/mmio/virtio_mmio.c wave=wave5 loc=980
export namespace pbsd::port::wave5::hbsd::src::sys::dev::virtio::mmio::virtio_mmio {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::virtio::mmio::virtio_mmio
