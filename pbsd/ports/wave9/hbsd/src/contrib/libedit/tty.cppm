export module pbsd.port.wave9.hbsd.src.contrib.libedit.tty;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libedit/tty.c
// void tty_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libedit/tty.c wave=wave9 loc=1372
export namespace pbsd::port::wave9::hbsd::src::contrib::libedit::tty {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libedit::tty
