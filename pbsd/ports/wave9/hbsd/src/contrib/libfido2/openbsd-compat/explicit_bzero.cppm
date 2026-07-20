export module pbsd.port.wave9.hbsd.src.contrib.libfido2.openbsd_compat.explicit_bzero;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libfido2/openbsd-compat/explicit_bzero.c
// void explicit_bzero_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libfido2/openbsd-compat/explicit_bzero.c wave=wave9 loc=57
export namespace pbsd::port::wave9::hbsd::src::contrib::libfido2::openbsd_compat::explicit_bzero {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libfido2::openbsd_compat::explicit_bzero
