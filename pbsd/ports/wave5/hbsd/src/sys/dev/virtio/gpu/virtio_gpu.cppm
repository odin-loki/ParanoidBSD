export module pbsd.port.wave5.hbsd.src.sys.dev.virtio.gpu.virtio_gpu;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/virtio/gpu/virtio_gpu.c
// void virtio_gpu_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/virtio/gpu/virtio_gpu.c wave=wave5 loc=734
export namespace pbsd::port::wave5::hbsd::src::sys::dev::virtio::gpu::virtio_gpu {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::virtio::gpu::virtio_gpu
