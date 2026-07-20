export module pbsd.port.wave9.hbsd.src.crypto.openssl.providers.fips.fips_entry;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/providers/fips/fips_entry.c
// void fips_entry_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/providers/fips/fips_entry.c wave=wave9 loc=19
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::providers::fips::fips_entry {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::providers::fips::fips_entry
