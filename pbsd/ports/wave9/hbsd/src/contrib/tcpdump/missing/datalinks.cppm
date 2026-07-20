export module pbsd.port.wave9.hbsd.src.contrib.tcpdump.missing.datalinks;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tcpdump/missing/datalinks.c
// void datalinks_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tcpdump/missing/datalinks.c wave=wave9 loc=60
export namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::missing::datalinks {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::missing::datalinks
