export module pbsd.port.wave9.hbsd.src.contrib.bearssl.src.ec.ec_secp384r1;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/bearssl/src/ec/ec_secp384r1.c
// void ec_secp384r1_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/bearssl/src/ec/ec_secp384r1.c wave=wave9 loc=57
export namespace pbsd::port::wave9::hbsd::src::contrib::bearssl::src::ec::ec_secp384r1 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::bearssl::src::ec::ec_secp384r1
