export module pbsd.port.wave9.hbsd.src.crypto.openssh.openbsd_compat.strtoul;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/openbsd-compat/strtoul.c
// void strtoul_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/openbsd-compat/strtoul.c wave=wave9 loc=108
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::strtoul {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::strtoul
