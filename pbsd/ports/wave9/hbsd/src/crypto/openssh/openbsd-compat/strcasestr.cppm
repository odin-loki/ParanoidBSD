export module pbsd.port.wave9.hbsd.src.crypto.openssh.openbsd_compat.strcasestr;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/openbsd-compat/strcasestr.c
// void strcasestr_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/openbsd-compat/strcasestr.c wave=wave9 loc=69
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::strcasestr {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::openbsd_compat::strcasestr
