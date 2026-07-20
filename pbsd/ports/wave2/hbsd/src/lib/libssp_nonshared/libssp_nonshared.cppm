export module pbsd.port.wave2.hbsd.src.lib.libssp_nonshared.libssp_nonshared;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libssp_nonshared/libssp_nonshared.c
// void libssp_nonshared_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libssp_nonshared/libssp_nonshared.c wave=wave2 loc=15
export namespace pbsd::port::wave2::hbsd::src::lib::libssp_nonshared::libssp_nonshared {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libssp_nonshared::libssp_nonshared
