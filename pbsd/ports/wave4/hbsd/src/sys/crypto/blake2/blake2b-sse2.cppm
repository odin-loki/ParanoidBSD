export module pbsd.port.wave4.hbsd.src.sys.crypto.blake2.blake2b_sse2;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/crypto/blake2/blake2b-sse2.c
// void blake2b-sse2_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/crypto/blake2/blake2b-sse2.c wave=wave4 loc=2
export namespace pbsd::port::wave4::hbsd::src::sys::crypto::blake2::blake2b_sse2 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::crypto::blake2::blake2b_sse2
