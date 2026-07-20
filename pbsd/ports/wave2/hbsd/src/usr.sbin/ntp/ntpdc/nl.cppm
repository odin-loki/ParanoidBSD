export module pbsd.port.wave2.hbsd.src.usr_sbin.ntp.ntpdc.nl;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/ntp/ntpdc/nl.c
// void nl_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/ntp/ntpdc/nl.c wave=wave2 loc=894
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::ntp::ntpdc::nl {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::ntp::ntpdc::nl
