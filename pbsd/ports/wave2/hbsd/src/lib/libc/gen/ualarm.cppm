export module pbsd.port.wave2.hbsd.src.lib.libc.gen.ualarm;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/gen/ualarm.c
// void ualarm_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/gen/ualarm.c wave=wave2 loc=56
export namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::ualarm {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::ualarm
