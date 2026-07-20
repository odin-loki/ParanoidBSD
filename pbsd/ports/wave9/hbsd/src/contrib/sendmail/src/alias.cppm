export module pbsd.port.wave9.hbsd.src.contrib.sendmail.src.alias;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/sendmail/src/alias.c
// void alias_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/sendmail/src/alias.c wave=wave9 loc=1108
export namespace pbsd::port::wave9::hbsd::src::contrib::sendmail::src::alias {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::sendmail::src::alias
