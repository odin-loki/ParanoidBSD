export module pbsd.port.wave4.hbsd.src.sys.crypto.siphash.siphash;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/crypto/siphash/siphash.c
// void siphash_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/crypto/siphash/siphash.c wave=wave4 loc=244
export namespace pbsd::port::wave4::hbsd::src::sys::crypto::siphash::siphash {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::crypto::siphash::siphash
