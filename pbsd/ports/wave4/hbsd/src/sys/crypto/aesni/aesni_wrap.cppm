export module pbsd.port.wave4.hbsd.src.sys.crypto.aesni.aesni_wrap;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/crypto/aesni/aesni_wrap.c
// void aesni_wrap_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/crypto/aesni/aesni_wrap.c wave=wave4 loc=484
export namespace pbsd::port::wave4::hbsd::src::sys::crypto::aesni::aesni_wrap {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::crypto::aesni::aesni_wrap
