export module pbsd.port.wave9.hbsd.src.crypto.openssh.ssherr;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/crypto/openssh/ssherr.c
// void ssherr_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/crypto/openssh/ssherr.c wave=wave9 loc=151
export namespace pbsd::port::wave9::hbsd::src::crypto::openssh::ssherr {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::crypto::openssh::ssherr
