export module pbsd.port.wave9.hbsd.src.contrib.libfido2.openbsd_compat.freezero;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libfido2/openbsd-compat/freezero.c
// void freezero_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libfido2/openbsd-compat/freezero.c wave=wave9 loc=30
export namespace pbsd::port::wave9::hbsd::src::contrib::libfido2::openbsd_compat::freezero {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libfido2::openbsd_compat::freezero
