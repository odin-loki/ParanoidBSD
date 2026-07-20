export module pbsd.port.wave9.hbsd.src.contrib.sendmail.libsm.ferror;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/sendmail/libsm/ferror.c
// void ferror_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/sendmail/libsm/ferror.c wave=wave9 loc=41
export namespace pbsd::port::wave9::hbsd::src::contrib::sendmail::libsm::ferror {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::sendmail::libsm::ferror
