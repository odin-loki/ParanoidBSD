export module pbsd.port.wave9.hbsd.src.crypto.openssh.openbsd_compat.bsd_snprintf;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/openbsd-compat/bsd-snprintf.c
// void bsd-snprintf_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/openbsd-compat/bsd-snprintf.c wave=wave9 loc=883
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::bsd_snprintf {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::bsd_snprintf
