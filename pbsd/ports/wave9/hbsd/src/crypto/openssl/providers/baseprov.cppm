export module pbsd.port.wave9.hbsd.src.crypto.openssl.providers.baseprov;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/providers/baseprov.c
// void baseprov_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/providers/baseprov.c wave=wave9 loc=189
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::providers::baseprov {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::providers::baseprov
