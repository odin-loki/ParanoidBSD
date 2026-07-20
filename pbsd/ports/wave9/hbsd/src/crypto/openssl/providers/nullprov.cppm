export module pbsd.port.wave9.hbsd.src.crypto.openssl.providers.nullprov;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/providers/nullprov.c
// void nullprov_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/providers/nullprov.c wave=wave9 loc=80
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::providers::nullprov {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::providers::nullprov
