export module pbsd.port.wave2.hbsd.src.usr_sbin.bhyve.iov;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/bhyve/iov.c
// void iov_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/bhyve/iov.c wave=wave2 loc=141
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::iov {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::iov
