export module pbsd.port.wave9.hbsd.src.contrib.tcpdump.print_dccp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tcpdump/print-dccp.c
// void print-dccp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tcpdump/print-dccp.c wave=wave9 loc=703
export namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_dccp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_dccp
