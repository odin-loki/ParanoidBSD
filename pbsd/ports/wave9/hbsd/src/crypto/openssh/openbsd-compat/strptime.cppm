export module pbsd.port.wave9.hbsd.src.crypto.openssh.openbsd_compat.strptime;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/openbsd-compat/strptime.c
// void strptime_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/openbsd-compat/strptime.c wave=wave9 loc=401
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::strptime {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::strptime
