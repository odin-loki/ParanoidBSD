export module pbsd.port.wave9.hbsd.src.contrib.tcpdump.print_ospf;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tcpdump/print-ospf.c
// void print-ospf_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tcpdump/print-ospf.c wave=wave9 loc=1182
export namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_ospf {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_ospf
