export module pbsd.port.wave2.hbsd.src.lib.libcrypt.crypt_md5;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libcrypt/crypt-md5.c
// void crypt-md5_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libcrypt/crypt-md5.c wave=wave2 loc=147
export namespace pbsd::port::wave2::hbsd::src::lib::libcrypt::crypt_md5 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libcrypt::crypt_md5
