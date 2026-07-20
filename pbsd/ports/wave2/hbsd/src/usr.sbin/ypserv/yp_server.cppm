export module pbsd.port.wave2.hbsd.src.usr_sbin.ypserv.yp_server;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/ypserv/yp_server.c
// void yp_server_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/ypserv/yp_server.c wave=wave2 loc=985
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::ypserv::yp_server {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::ypserv::yp_server
