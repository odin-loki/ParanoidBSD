export module pbsd.port.wave2.hbsd.src.lib.libc.gen.raise;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/gen/raise.c
// void raise_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/gen/raise.c wave=wave2 loc=49
export namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::raise {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::raise
