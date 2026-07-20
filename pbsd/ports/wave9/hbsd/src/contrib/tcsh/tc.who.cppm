export module pbsd.port.wave9.hbsd.src.contrib.tcsh.tc_who;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/tcsh/tc.who.c
// void tc.who_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/tcsh/tc.who.c wave=wave9 loc=645
export namespace pbsd::port::wave9::hbsd::src::contrib::tcsh::tc_who {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::tcsh::tc_who
