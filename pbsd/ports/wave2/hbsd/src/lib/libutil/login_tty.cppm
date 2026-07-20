export module pbsd.port.wave2.hbsd.src.lib.libutil.login_tty;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libutil/login_tty.c
// void login_tty_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libutil/login_tty.c wave=wave2 loc=56
export namespace pbsd::port::wave2::hbsd::src::lib::libutil::login_tty {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libutil::login_tty
