export module pbsd.port.wave2.hbsd.src.lib.libgssapi.gss_sign;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libgssapi/gss_sign.c
// void gss_sign_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libgssapi/gss_sign.c wave=wave2 loc=41
export namespace pbsd::port::wave2::hbsd::src::lib::libgssapi::gss_sign {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libgssapi::gss_sign
