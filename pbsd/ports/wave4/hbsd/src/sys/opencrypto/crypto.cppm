export module pbsd.port.wave4.hbsd.src.sys.opencrypto.crypto;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/opencrypto/crypto.c
// void crypto_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/opencrypto/crypto.c wave=wave4 loc=2001
export namespace pbsd::port::wave4::hbsd::src::sys::opencrypto::crypto {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::opencrypto::crypto
