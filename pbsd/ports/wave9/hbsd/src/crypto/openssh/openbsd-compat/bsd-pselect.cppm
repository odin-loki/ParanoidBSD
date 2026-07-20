export module pbsd.port.wave9.hbsd.src.crypto.openssh.openbsd_compat.bsd_pselect;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/openbsd-compat/bsd-pselect.c
// void bsd-pselect_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/openbsd-compat/bsd-pselect.c wave=wave9 loc=201
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::bsd_pselect {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::bsd_pselect
