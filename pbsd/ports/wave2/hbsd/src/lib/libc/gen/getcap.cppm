export module pbsd.port.wave2.hbsd.src.lib.libc.gen.getcap;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/gen/getcap.c
// void getcap_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/gen/getcap.c wave=wave2 loc=1051
export namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::getcap {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::getcap
