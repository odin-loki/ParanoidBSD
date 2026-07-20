export module pbsd.port.wave4.hbsd.src.sys.contrib.libsodium.src.libsodium.crypto_secretbox.crypto_secretbox_easy;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/libsodium/src/libsodium/crypto_secretbox/crypto_secretbox_easy.c
// void crypto_secretbox_easy_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/libsodium/src/libsodium/crypto_secretbox/crypto_secretbox_easy.c wave=wave4 loc=144
export namespace pbsd::port::wave4::hbsd::src::sys::contrib::libsodium::src::libsodium::crypto_secretbox::crypto_secretbox_easy {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::contrib::libsodium::src::libsodium::crypto_secretbox::crypto_secretbox_easy
