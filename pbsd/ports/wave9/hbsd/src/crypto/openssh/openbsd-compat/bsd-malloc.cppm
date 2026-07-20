export module pbsd.port.wave9.hbsd.src.crypto.openssh.openbsd_compat.bsd_malloc;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/openbsd-compat/bsd-malloc.c
// void bsd-malloc_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/openbsd-compat/bsd-malloc.c wave=wave9 loc=57
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::bsd_malloc {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::bsd_malloc
