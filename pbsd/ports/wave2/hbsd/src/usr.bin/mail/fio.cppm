export module pbsd.port.wave2.hbsd.src.usr_bin.mail.fio;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/mail/fio.c
// void fio_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/mail/fio.c wave=wave2 loc=445
export namespace pbsd::port::wave2::hbsd::src::usr_bin::mail::fio {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::mail::fio
