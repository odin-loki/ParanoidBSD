export module pbsd.port.wave2.hbsd.src.lib.libc.gen.getutxent;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/gen/getutxent.c
// void getutxent_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/gen/getutxent.c wave=wave2 loc=240
export namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::getutxent {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::getutxent
