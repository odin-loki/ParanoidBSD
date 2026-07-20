export module pbsd.port.wave9.hbsd.src.contrib.tcpdump.l2vpn;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tcpdump/l2vpn.c
// void l2vpn_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tcpdump/l2vpn.c wave=wave9 loc=93
export namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::l2vpn {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::l2vpn
