export module pbsd.port.wave9.hbsd.src.crypto.openssh.xmss_wots;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/xmss_wots.c
// void xmss_wots_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/xmss_wots.c wave=wave9 loc=192
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::xmss_wots {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::xmss_wots
