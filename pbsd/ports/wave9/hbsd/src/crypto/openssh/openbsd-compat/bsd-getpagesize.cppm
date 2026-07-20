export module pbsd.port.wave9.hbsd.src.crypto.openssh.openbsd_compat.bsd_getpagesize;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/openbsd-compat/bsd-getpagesize.c
// void bsd-getpagesize_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/openbsd-compat/bsd-getpagesize.c wave=wave9 loc=25
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::bsd_getpagesize {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::bsd_getpagesize
