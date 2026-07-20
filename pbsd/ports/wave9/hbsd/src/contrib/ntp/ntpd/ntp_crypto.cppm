export module pbsd.port.wave9.hbsd.src.contrib.ntp.ntpd.ntp_crypto;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ntp/ntpd/ntp_crypto.c
// void ntp_crypto_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ntp/ntpd/ntp_crypto.c wave=wave9 loc=4162
export namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::ntp_crypto {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ntp::ntpd::ntp_crypto
