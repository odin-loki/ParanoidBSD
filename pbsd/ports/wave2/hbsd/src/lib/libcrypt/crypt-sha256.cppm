export module pbsd.port.wave2.hbsd.src.lib.libcrypt.crypt_sha256;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libcrypt/crypt-sha256.c
// void crypt-sha256_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libcrypt/crypt-sha256.c wave=wave2 loc=423
export namespace pbsd::port::wave2::hbsd::src::lib::libcrypt::crypt_sha256 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libcrypt::crypt_sha256
