export module pbsd.port.wave9.hbsd.src.crypto.openssh.openbsd_compat.arc4random;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/openbsd-compat/arc4random.c
// void arc4random_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/openbsd-compat/arc4random.c wave=wave9 loc=254
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::arc4random {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::arc4random
