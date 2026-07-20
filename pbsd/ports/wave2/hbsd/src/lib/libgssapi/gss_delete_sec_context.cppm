export module pbsd.port.wave2.hbsd.src.lib.libgssapi.gss_delete_sec_context;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libgssapi/gss_delete_sec_context.c
// void gss_delete_sec_context_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libgssapi/gss_delete_sec_context.c wave=wave2 loc=62
export namespace pbsd::port::wave2::hbsd::src::lib::libgssapi::gss_delete_sec_context {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libgssapi::gss_delete_sec_context
