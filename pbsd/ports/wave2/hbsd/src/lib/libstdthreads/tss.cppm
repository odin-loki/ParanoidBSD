export module pbsd.port.wave2.hbsd.src.lib.libstdthreads.tss;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libstdthreads/tss.c
// void tss_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libstdthreads/tss.c wave=wave2 loc=67
export namespace pbsd::port::wave2::hbsd::src::lib::libstdthreads::tss {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libstdthreads::tss
