export module pbsd.port.wave4.hbsd.src.sys.crypto.aesni.aesni;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/crypto/aesni/aesni.c
// void aesni_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/crypto/aesni/aesni.c wave=wave4 loc=868
export namespace pbsd::port::wave4::hbsd::src::sys::crypto::aesni::aesni {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::crypto::aesni::aesni
