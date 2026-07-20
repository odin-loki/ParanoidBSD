export module pbsd.port.wave2.hbsd.src.lib.libc.gen.ttyslot;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/gen/ttyslot.c
// void ttyslot_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/gen/ttyslot.c wave=wave2 loc=43
export namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::ttyslot {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::ttyslot
