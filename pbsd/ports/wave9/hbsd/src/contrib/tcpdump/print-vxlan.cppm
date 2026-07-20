export module pbsd.port.wave9.hbsd.src.contrib.tcpdump.print_vxlan;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tcpdump/print-vxlan.c
// void print-vxlan_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tcpdump/print-vxlan.c wave=wave9 loc=81
export namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_vxlan {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_vxlan
