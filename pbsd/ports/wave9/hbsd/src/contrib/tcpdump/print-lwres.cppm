export module pbsd.port.wave9.hbsd.src.contrib.tcpdump.print_lwres;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tcpdump/print-lwres.c
// void print-lwres_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tcpdump/print-lwres.c wave=wave9 loc=555
export namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_lwres {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_lwres
