export module pbsd.port.wave4.hbsd.src.sys.contrib.libsodium.src.libsodium.crypto_sign.crypto_sign;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/libsodium/src/libsodium/crypto_sign/crypto_sign.c
// void crypto_sign_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/libsodium/src/libsodium/crypto_sign/crypto_sign.c wave=wave4 loc=115
export namespace pbsd::port::wave4::hbsd::src::sys::contrib::libsodium::src::libsodium::crypto_sign::crypto_sign {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::contrib::libsodium::src::libsodium::crypto_sign::crypto_sign
