export module pbsd.port.wave9.hbsd.src.contrib.tcpdump.print_vxlan_gpe;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tcpdump/print-vxlan-gpe.c
// void print-vxlan-gpe_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tcpdump/print-vxlan-gpe.c wave=wave9 loc=122
export namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_vxlan_gpe {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_vxlan_gpe
