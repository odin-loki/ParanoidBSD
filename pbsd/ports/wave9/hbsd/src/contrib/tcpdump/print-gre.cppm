export module pbsd.port.wave9.hbsd.src.contrib.tcpdump.print_gre;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tcpdump/print-gre.c
// void print-gre_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tcpdump/print-gre.c wave=wave9 loc=412
export namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_gre {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_gre
