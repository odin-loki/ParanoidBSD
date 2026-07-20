export module pbsd.port.wave9.hbsd.src.crypto.openssh.openbsd_compat.readpassphrase;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/openbsd-compat/readpassphrase.c
// void readpassphrase_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/openbsd-compat/readpassphrase.c wave=wave9 loc=211
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::readpassphrase {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::readpassphrase
