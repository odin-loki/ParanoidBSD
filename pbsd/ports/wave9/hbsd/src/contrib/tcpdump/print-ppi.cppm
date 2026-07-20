export module pbsd.port.wave9.hbsd.src.contrib.tcpdump.print_ppi;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tcpdump/print-ppi.c
// void print-ppi_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tcpdump/print-ppi.c wave=wave9 loc=129
export namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_ppi {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_ppi
