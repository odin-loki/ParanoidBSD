export module pbsd.port.wave2.hbsd.src.lib.libgssapi.gss_names;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libgssapi/gss_names.c
// void gss_names_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libgssapi/gss_names.c wave=wave2 loc=260
export namespace pbsd::port::wave2::hbsd::src::lib::libgssapi::gss_names {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libgssapi::gss_names
