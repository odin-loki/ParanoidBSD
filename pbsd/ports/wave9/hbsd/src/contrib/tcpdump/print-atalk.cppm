export module pbsd.port.wave9.hbsd.src.contrib.tcpdump.print_atalk;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tcpdump/print-atalk.c
// void print-atalk_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tcpdump/print-atalk.c wave=wave9 loc=699
export namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_atalk {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::print_atalk
