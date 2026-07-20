export module pbsd.port.wave9.hbsd.src.contrib.tcpdump.print_sflow;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tcpdump/print-sflow.c
// void print-sflow_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tcpdump/print-sflow.c wave=wave9 loc=960
export namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_sflow {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_sflow
