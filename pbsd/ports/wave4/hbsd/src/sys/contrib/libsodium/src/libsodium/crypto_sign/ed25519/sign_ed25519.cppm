export module pbsd.port.wave4.hbsd.src.sys.contrib.libsodium.src.libsodium.crypto_sign.ed25519.sign_ed25519;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/libsodium/src/libsodium/crypto_sign/ed25519/sign_ed25519.c
// void sign_ed25519_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/libsodium/src/libsodium/crypto_sign/ed25519/sign_ed25519.c wave=wave4 loc=97
export namespace pbsd::port::wave4::hbsd::src::sys::contrib::libsodium::src::libsodium::crypto_sign::ed25519::sign_ed25519 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::contrib::libsodium::src::libsodium::crypto_sign::ed25519::sign_ed25519
