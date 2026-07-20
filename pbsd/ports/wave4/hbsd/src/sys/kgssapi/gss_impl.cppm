export module pbsd.port.wave4.hbsd.src.sys.kgssapi.gss_impl;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kgssapi/gss_impl.c
// void gss_impl_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kgssapi/gss_impl.c wave=wave4 loc=349
export namespace pbsd::port::wave4::hbsd::src::sys::kgssapi::gss_impl {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kgssapi::gss_impl
