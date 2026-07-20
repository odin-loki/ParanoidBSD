export module pbsd.port.wave4.hbsd.src.sys.kgssapi.gss_import_name;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kgssapi/gss_import_name.c
// void gss_import_name_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kgssapi/gss_import_name.c wave=wave4 loc=83
export namespace pbsd::port::wave4::hbsd::src::sys::kgssapi::gss_import_name {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kgssapi::gss_import_name
