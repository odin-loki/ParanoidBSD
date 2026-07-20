export module pbsd.port.wave9.hbsd.src.crypto.openssh.openbsd_compat.glob;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/openbsd-compat/glob.c
// void glob_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/openbsd-compat/glob.c wave=wave9 loc=1079
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::glob {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::glob
