export module pbsd.port.wave2.hbsd.src.lib.libc.gen.getlogin;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/gen/getlogin.c
// void getlogin_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/gen/getlogin.c wave=wave2 loc=84
export namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::getlogin {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::getlogin
