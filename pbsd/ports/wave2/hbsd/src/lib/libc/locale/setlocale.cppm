export module pbsd.port.wave2.hbsd.src.lib.libc.locale.setlocale;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/locale/setlocale.c
// void setlocale_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/locale/setlocale.c wave=wave2 loc=322
export namespace pbsd::port::wave2::hbsd::src::lib::libc::locale::setlocale {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::locale::setlocale
