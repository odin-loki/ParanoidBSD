export module pbsd.port.wave9.hbsd.src.tests.sys.kern.tty.readsz;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/tests/sys/kern/tty/readsz.c
// void readsz_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/tests/sys/kern/tty/readsz.c wave=wave9 loc=130
export namespace pbsd::port::wave9::hbsd::src::tests::sys::kern::tty::readsz {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::tests::sys::kern::tty::readsz
