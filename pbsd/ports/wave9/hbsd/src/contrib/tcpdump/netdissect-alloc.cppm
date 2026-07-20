export module pbsd.port.wave9.hbsd.src.contrib.tcpdump.netdissect_alloc;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tcpdump/netdissect-alloc.c
// void netdissect-alloc_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tcpdump/netdissect-alloc.c wave=wave9 loc=62
export namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::netdissect_alloc {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::netdissect_alloc
