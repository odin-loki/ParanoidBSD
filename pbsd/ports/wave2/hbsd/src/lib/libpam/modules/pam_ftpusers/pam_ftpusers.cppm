export module pbsd.port.wave2.hbsd.src.lib.libpam.modules.pam_ftpusers.pam_ftpusers;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libpam/modules/pam_ftpusers/pam_ftpusers.c
// void pam_ftpusers_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libpam/modules/pam_ftpusers/pam_ftpusers.c wave=wave2 loc=115
export namespace pbsd::port::wave2::hbsd::src::lib::libpam::modules::pam_ftpusers::pam_ftpusers {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libpam::modules::pam_ftpusers::pam_ftpusers
