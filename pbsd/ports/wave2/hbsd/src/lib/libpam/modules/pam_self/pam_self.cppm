export module pbsd.port.wave2.hbsd.src.lib.libpam.modules.pam_self.pam_self;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libpam/modules/pam_self/pam_self.c
// void pam_self_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libpam/modules/pam_self/pam_self.c wave=wave2 loc=89
export namespace pbsd::port::wave2::hbsd::src::lib::libpam::modules::pam_self::pam_self {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libpam::modules::pam_self::pam_self
