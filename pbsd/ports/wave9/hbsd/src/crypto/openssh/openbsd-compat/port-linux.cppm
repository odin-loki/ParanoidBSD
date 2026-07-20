export module pbsd.port.wave9.hbsd.src.crypto.openssh.openbsd_compat.port_linux;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/openbsd-compat/port-linux.c
// void port-linux_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/openbsd-compat/port-linux.c wave=wave9 loc=420
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::port_linux {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::port_linux
