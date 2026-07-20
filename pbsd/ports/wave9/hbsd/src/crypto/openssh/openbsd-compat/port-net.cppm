export module pbsd.port.wave9.hbsd.src.crypto.openssh.openbsd_compat.port_net;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/openbsd-compat/port-net.c
// void port-net_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/openbsd-compat/port-net.c wave=wave9 loc=378
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::port_net {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::port_net
