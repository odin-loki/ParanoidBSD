export module pbsd.port.wave2.hbsd.src.lib.libpam.modules.pam_deny.pam_deny;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libpam/modules/pam_deny/pam_deny.c
// void pam_deny_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libpam/modules/pam_deny/pam_deny.c wave=wave2 loc=93
export namespace pbsd::port::wave2::hbsd::src::lib::libpam::modules::pam_deny::pam_deny {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libpam::modules::pam_deny::pam_deny
