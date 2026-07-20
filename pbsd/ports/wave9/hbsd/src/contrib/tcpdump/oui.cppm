export module pbsd.port.wave9.hbsd.src.contrib.tcpdump.oui;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tcpdump/oui.c
// void oui_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tcpdump/oui.c wave=wave9 loc=129
export namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::oui {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tcpdump::oui
