export module pbsd.port.wave2.hbsd.src.usr_sbin.bhyve.aarch64.vmexit;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/bhyve/aarch64/vmexit.c
// void vmexit_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/bhyve/aarch64/vmexit.c wave=wave2 loc=305
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::aarch64::vmexit {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::aarch64::vmexit
