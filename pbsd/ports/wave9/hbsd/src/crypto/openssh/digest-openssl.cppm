export module pbsd.port.wave9.hbsd.src.crypto.openssh.digest_openssl;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/digest-openssl.c
// void digest-openssl_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/digest-openssl.c wave=wave9 loc=207
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::digest_openssl {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::digest_openssl
