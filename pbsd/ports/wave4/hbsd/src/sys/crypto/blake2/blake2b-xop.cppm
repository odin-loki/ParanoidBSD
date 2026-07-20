export module pbsd.port.wave4.hbsd.src.sys.crypto.blake2.blake2b_xop;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/crypto/blake2/blake2b-xop.c
// void blake2b-xop_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/crypto/blake2/blake2b-xop.c wave=wave4 loc=2
export namespace pbsd::port::wave4::hbsd::src::sys::crypto::blake2::blake2b_xop {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::crypto::blake2::blake2b_xop
