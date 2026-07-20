export module pbsd.port.wave2.hbsd.src.lib.libgssapi.gss_verify;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libgssapi/gss_verify.c
// void gss_verify_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libgssapi/gss_verify.c wave=wave2 loc=42
export namespace pbsd::port::wave2::hbsd::src::lib::libgssapi::gss_verify {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libgssapi::gss_verify
