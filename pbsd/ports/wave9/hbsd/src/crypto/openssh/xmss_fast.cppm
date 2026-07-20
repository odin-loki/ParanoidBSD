export module pbsd.port.wave9.hbsd.src.crypto.openssh.xmss_fast;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/xmss_fast.c
// void xmss_fast_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/xmss_fast.c wave=wave9 loc=1106
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::xmss_fast {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::xmss_fast
