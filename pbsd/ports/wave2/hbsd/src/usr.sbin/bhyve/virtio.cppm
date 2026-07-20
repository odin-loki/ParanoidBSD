export module pbsd.port.wave2.hbsd.src.usr_sbin.bhyve.virtio;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/bhyve/virtio.c
// void virtio_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/bhyve/virtio.c wave=wave2 loc=968
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::virtio {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::virtio
