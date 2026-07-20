export module pbsd.port.wave2.hbsd.src.lib.libpam.modules.pam_rootok.pam_rootok;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libpam/modules/pam_rootok/pam_rootok.c
// void pam_rootok_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libpam/modules/pam_rootok/pam_rootok.c wave=wave2 loc=73
export namespace pbsd::port::wave2::hbsd::src::lib::libpam::modules::pam_rootok::pam_rootok {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libpam::modules::pam_rootok::pam_rootok
