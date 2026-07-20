export module pbsd.port.wave9.hbsd.src.crypto.openssl.crypto.ec.ecp_sm2p256;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/crypto/ec/ecp_sm2p256.c
// void ecp_sm2p256_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/crypto/ec/ecp_sm2p256.c wave=wave9 loc=694
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::crypto::ec::ecp_sm2p256 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::crypto::ec::ecp_sm2p256
