export module pbsd.port.wave2.hbsd.src.lib.libpam.modules.pam_ssh.pam_ssh;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libpam/modules/pam_ssh/pam_ssh.c
// void pam_ssh_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libpam/modules/pam_ssh/pam_ssh.c wave=wave2 loc=442
export namespace pbsd::port::wave2::hbsd::src::lib::libpam::modules::pam_ssh::pam_ssh {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libpam::modules::pam_ssh::pam_ssh
