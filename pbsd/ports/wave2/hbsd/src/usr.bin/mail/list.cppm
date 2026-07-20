export module pbsd.port.wave2.hbsd.src.usr_bin.mail.list;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/mail/list.c
// void list_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/mail/list.c wave=wave2 loc=810
export namespace pbsd::port::wave2::hbsd::src::usr_bin::mail::list {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::mail::list
