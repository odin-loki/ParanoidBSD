export module pbsd.port.wave2.hbsd.src.usr_bin.localedef.monetary;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/localedef/monetary.c
// void monetary_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/localedef/monetary.c wave=wave2 loc=209
export namespace pbsd::port::wave2::hbsd::src::usr_bin::localedef::monetary {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::localedef::monetary
