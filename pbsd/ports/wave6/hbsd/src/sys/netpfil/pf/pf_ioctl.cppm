export module pbsd.port.wave6.hbsd.src.sys.netpfil.pf.pf_ioctl;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netpfil/pf/pf_ioctl.c
// void pf_ioctl_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netpfil/pf/pf_ioctl.c wave=wave6 loc=7250
export namespace pbsd::port::wave6::hbsd::src::sys::netpfil::pf::pf_ioctl {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netpfil::pf::pf_ioctl
