export module pbsd.port.wave9.hbsd.src.crypto.openssl.crypto.md5.md5_dgst;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssl/crypto/md5/md5_dgst.c
// void md5_dgst_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssl/crypto/md5/md5_dgst.c wave=wave9 loc=170
export namespace pbsd::port::wave9::hbsd::src::crypto::openssl::crypto::md5::md5_dgst {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssl::crypto::md5::md5_dgst
