export module pbsd.port.wave9.hbsd.src.crypto.openssh.openbsd_compat.bsd_err;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/openbsd-compat/bsd-err.c
// void bsd-err_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/openbsd-compat/bsd-err.c wave=wave9 loc=77
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::bsd_err {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::bsd_err
