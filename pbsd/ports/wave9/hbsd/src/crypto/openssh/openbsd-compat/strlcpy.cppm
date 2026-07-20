export module pbsd.port.wave9.hbsd.src.crypto.openssh.openbsd_compat.strlcpy;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/openbsd-compat/strlcpy.c
// void strlcpy_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/openbsd-compat/strlcpy.c wave=wave9 loc=58
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::strlcpy {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::strlcpy
