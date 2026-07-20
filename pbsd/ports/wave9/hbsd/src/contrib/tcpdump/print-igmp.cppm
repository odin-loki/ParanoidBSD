export module pbsd.port.wave9.hbsd.src.contrib.tcpdump.print_igmp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tcpdump/print-igmp.c
// void print-igmp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tcpdump/print-igmp.c wave=wave9 loc=299
export namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_igmp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_igmp
