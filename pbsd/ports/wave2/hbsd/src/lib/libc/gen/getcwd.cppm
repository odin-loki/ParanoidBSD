export module pbsd.port.wave2.hbsd.src.lib.libc.gen.getcwd;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/gen/getcwd.c
// void getcwd_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/gen/getcwd.c wave=wave2 loc=227
export namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::getcwd {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::getcwd
