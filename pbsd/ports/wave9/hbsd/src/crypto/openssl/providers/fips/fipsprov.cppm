export module pbsd.port.wave9.hbsd.src.crypto.openssl.providers.fips.fipsprov;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/providers/fips/fipsprov.c
// void fipsprov_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/providers/fips/fipsprov.c wave=wave9 loc=1177
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::providers::fips::fipsprov {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::providers::fips::fipsprov
