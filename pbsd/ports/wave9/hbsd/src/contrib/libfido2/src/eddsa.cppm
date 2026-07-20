export module pbsd.port.wave9.hbsd.src.contrib.libfido2.src.eddsa;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libfido2/src/eddsa.c
// void eddsa_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libfido2/src/eddsa.c wave=wave9 loc=232
export namespace pbsd::port::wave9::hbsd::src::contrib::libfido2::src::eddsa {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libfido2::src::eddsa
