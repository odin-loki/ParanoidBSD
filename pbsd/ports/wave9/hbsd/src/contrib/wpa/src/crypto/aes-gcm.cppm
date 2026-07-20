export module pbsd.port.wave9.hbsd.src.contrib.wpa.src.crypto.aes_gcm;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/wpa/src/crypto/aes-gcm.c
// void aes-gcm_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/wpa/src/crypto/aes-gcm.c wave=wave9 loc=327
export namespace pbsd::port::wave9::hbsd::src::contrib::wpa::src::crypto::aes_gcm {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::wpa::src::crypto::aes_gcm
