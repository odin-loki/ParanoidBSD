export module pbsd.port.wave4.hbsd.src.sys.contrib.libsodium.src.libsodium.crypto_auth.crypto_auth;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/libsodium/src/libsodium/crypto_auth/crypto_auth.c
// void crypto_auth_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/libsodium/src/libsodium/crypto_auth/crypto_auth.c wave=wave4 loc=41
export namespace pbsd::port::wave4::hbsd::src::sys::contrib::libsodium::src::libsodium::crypto_auth::crypto_auth {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::contrib::libsodium::src::libsodium::crypto_auth::crypto_auth
