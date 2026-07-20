export module pbsd.port.wave2.hbsd.src.lib.libgssapi.gss_duplicate_name;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libgssapi/gss_duplicate_name.c
// void gss_duplicate_name_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libgssapi/gss_duplicate_name.c wave=wave2 loc=100
export namespace pbsd::port::wave2::hbsd::src::lib::libgssapi::gss_duplicate_name {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libgssapi::gss_duplicate_name
