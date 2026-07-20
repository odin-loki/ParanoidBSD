export module pbsd.port.wave9.hbsd.src.crypto.openssl.apps.dgst;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/apps/dgst.c
// void dgst_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/apps/dgst.c wave=wave9 loc=761
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::apps::dgst {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::apps::dgst
