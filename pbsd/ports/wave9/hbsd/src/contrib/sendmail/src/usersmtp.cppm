export module pbsd.port.wave9.hbsd.src.contrib.sendmail.src.usersmtp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/sendmail/src/usersmtp.c
// void usersmtp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/sendmail/src/usersmtp.c wave=wave9 loc=3557
export namespace pbsd::port::wave9::hbsd::src::contrib::sendmail::src::usersmtp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::sendmail::src::usersmtp
