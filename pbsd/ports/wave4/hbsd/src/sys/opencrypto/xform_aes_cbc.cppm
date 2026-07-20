export module pbsd.port.wave4.hbsd.src.sys.opencrypto.xform_aes_cbc;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/opencrypto/xform_aes_cbc.c
// void xform_aes_cbc_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/opencrypto/xform_aes_cbc.c wave=wave4 loc=167
export namespace pbsd::port::wave4::hbsd::src::sys::opencrypto::xform_aes_cbc {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::opencrypto::xform_aes_cbc
