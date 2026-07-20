export module pbsd.port.wave4.hbsd.src.sys.crypto.blake2.blake2_sw;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/crypto/blake2/blake2-sw.c
// void blake2-sw_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/crypto/blake2/blake2-sw.c wave=wave4 loc=151
export namespace pbsd::port::wave4::hbsd::src::sys::crypto::blake2::blake2_sw {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::crypto::blake2::blake2_sw
