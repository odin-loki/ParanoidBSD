export module pbsd.port.wave9.hbsd.src.crypto.openssh.ssh_dss;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/ssh-dss.c
// void ssh-dss_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/ssh-dss.c wave=wave9 loc=457
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::ssh_dss {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::ssh_dss
