export module pbsd.port.wave9.hbsd.src.crypto.openssl.apps.genrsa;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/apps/genrsa.c
// void genrsa_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/apps/genrsa.c wave=wave9 loc=258
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::apps::genrsa {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::apps::genrsa
