export module pbsd.port.wave4.hbsd.src.sys.crypto.aesni.aesni_ghash;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/crypto/aesni/aesni_ghash.c
// void aesni_ghash_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/crypto/aesni/aesni_ghash.c wave=wave4 loc=808
export namespace pbsd::port::wave4::hbsd::src::sys::crypto::aesni::aesni_ghash {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::crypto::aesni::aesni_ghash
