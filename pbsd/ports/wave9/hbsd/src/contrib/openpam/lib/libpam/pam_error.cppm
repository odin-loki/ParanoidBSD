export module pbsd.port.wave9.hbsd.src.contrib.openpam.lib.libpam.pam_error;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/openpam/lib/libpam/pam_error.c
// void pam_error_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/openpam/lib/libpam/pam_error.c wave=wave9 loc=87
export namespace pbsd::port::wave9::hbsd::src::contrib::openpam::lib::libpam::pam_error {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::openpam::lib::libpam::pam_error
