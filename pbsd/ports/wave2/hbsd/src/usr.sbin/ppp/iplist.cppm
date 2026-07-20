export module pbsd.port.wave2.hbsd.src.usr_sbin.ppp.iplist;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/ppp/iplist.c
// void iplist_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/ppp/iplist.c wave=wave2 loc=225
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::ppp::iplist {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::ppp::iplist
