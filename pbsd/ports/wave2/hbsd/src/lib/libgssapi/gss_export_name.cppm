export module pbsd.port.wave2.hbsd.src.lib.libgssapi.gss_export_name;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libgssapi/gss_export_name.c
// void gss_export_name_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libgssapi/gss_export_name.c wave=wave2 loc=58
export namespace pbsd::port::wave2::hbsd::src::lib::libgssapi::gss_export_name {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libgssapi::gss_export_name
