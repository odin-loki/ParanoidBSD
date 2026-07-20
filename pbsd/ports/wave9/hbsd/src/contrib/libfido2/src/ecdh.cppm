export module pbsd.port.wave9.hbsd.src.contrib.libfido2.src.ecdh;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libfido2/src/ecdh.c
// void ecdh_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libfido2/src/ecdh.c wave=wave9 loc=208
export namespace pbsd::port::wave9::hbsd::src::contrib::libfido2::src::ecdh {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libfido2::src::ecdh
