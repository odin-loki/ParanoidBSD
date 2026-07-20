export module pbsd.port.wave9.hbsd.src.contrib.ntp.ntpd.ntp_scanner;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ntp/ntpd/ntp_scanner.c
// void ntp_scanner_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ntp/ntpd/ntp_scanner.c wave=wave9 loc=948
export namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::ntp_scanner {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::ntp_scanner
