export module pbsd.port.wave4.hbsd.src.sys.contrib.libsodium.src.libsodium.crypto_aead.aes256gcm.aesni.aead_aes256gcm_aesni;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/libsodium/src/libsodium/crypto_aead/aes256gcm/aesni/aead_aes256gcm_aesni.c
// void aead_aes256gcm_aesni_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/libsodium/src/libsodium/crypto_aead/aes256gcm/aesni/aead_aes256gcm_aesni.c wave=wave4 loc=1079
export namespace pbsd::port::wave4::hbsd::src::sys::contrib::libsodium::src::libsodium::crypto_aead::aes256gcm::aesni::aead_aes256gcm_aesni {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::contrib::libsodium::src::libsodium::crypto_aead::aes256gcm::aesni::aead_aes256gcm_aesni
