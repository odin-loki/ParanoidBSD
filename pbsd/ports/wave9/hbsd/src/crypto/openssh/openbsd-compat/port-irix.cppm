export module pbsd.port.wave9.hbsd.src.crypto.openssh.openbsd_compat.port_irix;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/openbsd-compat/port-irix.c
// void port-irix_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/openbsd-compat/port-irix.c wave=wave9 loc=92
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::port_irix {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::port_irix
