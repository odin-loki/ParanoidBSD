export module pbsd.port.wave2.hbsd.src.usr_bin.locale.locale;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/locale/locale.c
// void locale_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/locale/locale.c wave=wave2 loc=936
export namespace pbsd::port::wave2::hbsd::src::usr_bin::locale::locale {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::locale::locale
