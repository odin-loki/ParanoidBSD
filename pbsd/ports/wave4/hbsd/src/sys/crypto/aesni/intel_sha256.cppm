export module pbsd.port.wave4.hbsd.src.sys.crypto.aesni.intel_sha256;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/crypto/aesni/intel_sha256.c
// void intel_sha256_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/crypto/aesni/intel_sha256.c wave=wave4 loc=276
export namespace pbsd::port::wave4::hbsd::src::sys::crypto::aesni::intel_sha256 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::crypto::aesni::intel_sha256
