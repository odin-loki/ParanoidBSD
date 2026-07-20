export module pbsd.port.wave2.hbsd.src.usr_sbin.bhyve.tpm_intf_crb;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/bhyve/tpm_intf_crb.c
// void tpm_intf_crb_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/bhyve/tpm_intf_crb.c wave=wave2 loc=627
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::tpm_intf_crb {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::tpm_intf_crb
