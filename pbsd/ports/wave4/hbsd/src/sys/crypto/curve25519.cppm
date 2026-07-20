export module pbsd.port.wave4.hbsd.src.sys.crypto.curve25519;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/crypto/curve25519.c
// void curve25519_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/crypto/curve25519.c wave=wave4 loc=46
export namespace pbsd::port::wave4::hbsd::src::sys::crypto::curve25519 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::crypto::curve25519
