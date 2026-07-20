export module pbsd.port.wave4.hbsd.src.sys.crypto.libsodium.randombytes;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/crypto/libsodium/randombytes.c
// void randombytes_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/crypto/libsodium/randombytes.c wave=wave4 loc=12
export namespace pbsd::port::wave4::hbsd::src::sys::crypto::libsodium::randombytes {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::crypto::libsodium::randombytes
