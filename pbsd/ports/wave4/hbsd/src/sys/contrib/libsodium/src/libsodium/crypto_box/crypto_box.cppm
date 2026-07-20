export module pbsd.port.wave4.hbsd.src.sys.contrib.libsodium.src.libsodium.crypto_box.crypto_box;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/libsodium/src/libsodium/crypto_box/crypto_box.c
// void crypto_box_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/libsodium/src/libsodium/crypto_box/crypto_box.c wave=wave4 loc=114
export namespace pbsd::port::wave4::hbsd::src::sys::contrib::libsodium::src::libsodium::crypto_box::crypto_box {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::contrib::libsodium::src::libsodium::crypto_box::crypto_box
