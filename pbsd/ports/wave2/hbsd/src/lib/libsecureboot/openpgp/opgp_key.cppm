export module pbsd.port.wave2.hbsd.src.lib.libsecureboot.openpgp.opgp_key;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libsecureboot/openpgp/opgp_key.c
// void opgp_key_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libsecureboot/openpgp/opgp_key.c wave=wave2 loc=413
export namespace pbsd::port::wave2::hbsd::src::lib::libsecureboot::openpgp::opgp_key {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libsecureboot::openpgp::opgp_key
