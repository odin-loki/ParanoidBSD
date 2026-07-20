export module pbsd.port.wave2.hbsd.src.usr_bin.mail.getname;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/mail/getname.c
// void getname_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/mail/getname.c wave=wave2 loc=63
export namespace pbsd::port::wave2::hbsd::src::usr_bin::mail::getname {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::mail::getname
