export module pbsd.port.wave9.hbsd.src.crypto.openssh.openbsd_compat.port_aix;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/openbsd-compat/port-aix.c
// void port-aix_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/openbsd-compat/port-aix.c wave=wave9 loc=483
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::port_aix {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::port_aix
