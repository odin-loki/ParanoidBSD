export module pbsd.port.wave4.hbsd.src.sys.opencrypto.ktls_ocf;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/opencrypto/ktls_ocf.c
// void ktls_ocf_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/opencrypto/ktls_ocf.c wave=wave4 loc=1188
export namespace pbsd::port::wave4::hbsd::src::sys::opencrypto::ktls_ocf {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::opencrypto::ktls_ocf
