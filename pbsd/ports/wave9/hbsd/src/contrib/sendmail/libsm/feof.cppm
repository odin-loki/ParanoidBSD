export module pbsd.port.wave9.hbsd.src.contrib.sendmail.libsm.feof;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/sendmail/libsm/feof.c
// void feof_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/sendmail/libsm/feof.c wave=wave9 loc=42
export namespace pbsd::port::wave9::hbsd::src::contrib::sendmail::libsm::feof {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::sendmail::libsm::feof
