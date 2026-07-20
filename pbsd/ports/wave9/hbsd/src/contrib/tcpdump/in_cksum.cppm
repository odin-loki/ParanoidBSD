export module pbsd.port.wave9.hbsd.src.contrib.tcpdump.in_cksum;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tcpdump/in_cksum.c
// void in_cksum_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tcpdump/in_cksum.c wave=wave9 loc=198
export namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::in_cksum {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::in_cksum
