export module pbsd.port.wave2.hbsd.src.lib.libgssapi.gss_pname_to_uid;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libgssapi/gss_pname_to_uid.c
// void gss_pname_to_uid_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libgssapi/gss_pname_to_uid.c wave=wave2 loc=70
export namespace pbsd::port::wave2::hbsd::src::lib::libgssapi::gss_pname_to_uid {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libgssapi::gss_pname_to_uid
