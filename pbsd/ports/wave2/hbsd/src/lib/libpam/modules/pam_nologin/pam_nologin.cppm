export module pbsd.port.wave2.hbsd.src.lib.libpam.modules.pam_nologin.pam_nologin;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libpam/modules/pam_nologin/pam_nologin.c
// void pam_nologin_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libpam/modules/pam_nologin/pam_nologin.c wave=wave2 loc=126
export namespace pbsd::port::wave2::hbsd::src::lib::libpam::modules::pam_nologin::pam_nologin {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libpam::modules::pam_nologin::pam_nologin
