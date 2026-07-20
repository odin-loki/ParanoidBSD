export module pbsd.port.wave2.hbsd.src.usr_sbin.bhyve.net_backend_slirp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/bhyve/net_backend_slirp.c
// void net_backend_slirp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/bhyve/net_backend_slirp.c wave=wave2 loc=293
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::net_backend_slirp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::net_backend_slirp
