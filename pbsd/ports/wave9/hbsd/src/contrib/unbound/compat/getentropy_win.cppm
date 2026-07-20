export module pbsd.port.wave9.hbsd.src.contrib.unbound.compat.getentropy_win;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/unbound/compat/getentropy_win.c
// void getentropy_win_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/unbound/compat/getentropy_win.c wave=wave9 loc=59
export namespace pbsd::port::wave9::hbsd::src::contrib::unbound::compat::getentropy_win {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::unbound::compat::getentropy_win
