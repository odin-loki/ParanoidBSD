export module pbsd.port.wave9.hbsd.src.crypto.openssl.crypto.provider_conf;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/crypto/provider_conf.c
// void provider_conf_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/crypto/provider_conf.c wave=wave9 loc=430
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::crypto::provider_conf {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::crypto::provider_conf
