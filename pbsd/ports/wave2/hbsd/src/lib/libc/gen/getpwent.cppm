export module pbsd.port.wave2.hbsd.src.lib.libc.gen.getpwent;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/gen/getpwent.c
// void getpwent_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/gen/getpwent.c wave=wave2 loc=2001
export namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::getpwent {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::getpwent
