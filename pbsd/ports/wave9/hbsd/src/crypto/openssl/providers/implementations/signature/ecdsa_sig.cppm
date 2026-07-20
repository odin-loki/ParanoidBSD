export module pbsd.port.wave9.hbsd.src.crypto.openssl.providers.implementations.signature.ecdsa_sig;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/providers/implementations/signature/ecdsa_sig.c
// void ecdsa_sig_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/providers/implementations/signature/ecdsa_sig.c wave=wave9 loc=1107
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::providers::implementations::signature::ecdsa_sig {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::providers::implementations::signature::ecdsa_sig
