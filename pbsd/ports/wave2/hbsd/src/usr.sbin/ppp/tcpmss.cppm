export module pbsd.port.wave2.hbsd.src.usr_sbin.ppp.tcpmss;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/ppp/tcpmss.c
// void tcpmss_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/ppp/tcpmss.c wave=wave2 loc=233
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::ppp::tcpmss {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::ppp::tcpmss
