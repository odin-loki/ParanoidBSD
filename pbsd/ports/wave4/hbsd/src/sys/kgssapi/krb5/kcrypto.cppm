export module pbsd.port.wave4.hbsd.src.sys.kgssapi.krb5.kcrypto;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kgssapi/krb5/kcrypto.c
// void kcrypto_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kgssapi/krb5/kcrypto.c wave=wave4 loc=261
export namespace pbsd::port::wave4::hbsd::src::sys::kgssapi::krb5::kcrypto {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kgssapi::krb5::kcrypto
