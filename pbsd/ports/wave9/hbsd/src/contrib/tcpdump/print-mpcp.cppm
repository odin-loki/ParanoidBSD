export module pbsd.port.wave9.hbsd.src.contrib.tcpdump.print_mpcp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tcpdump/print-mpcp.c
// void print-mpcp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tcpdump/print-mpcp.c wave=wave9 loc=233
export namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_mpcp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_mpcp
