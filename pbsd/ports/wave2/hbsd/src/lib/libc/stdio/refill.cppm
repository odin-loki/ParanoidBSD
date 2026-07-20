export module pbsd.port.wave2.hbsd.src.lib.libc.stdio.refill;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdio/refill.c
// void refill_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdio/refill.c wave=wave2 loc=142
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::refill {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::refill
