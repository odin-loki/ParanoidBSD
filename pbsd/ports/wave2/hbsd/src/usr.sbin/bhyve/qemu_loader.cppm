export module pbsd.port.wave2.hbsd.src.usr_sbin.bhyve.qemu_loader;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/bhyve/qemu_loader.c
// void qemu_loader_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/bhyve/qemu_loader.c wave=wave2 loc=274
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::qemu_loader {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::qemu_loader
