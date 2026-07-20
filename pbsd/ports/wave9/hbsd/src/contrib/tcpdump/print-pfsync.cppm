export module pbsd.port.wave9.hbsd.src.contrib.tcpdump.print_pfsync;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tcpdump/print-pfsync.c
// void print-pfsync_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tcpdump/print-pfsync.c wave=wave9 loc=484
export namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_pfsync {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_pfsync
