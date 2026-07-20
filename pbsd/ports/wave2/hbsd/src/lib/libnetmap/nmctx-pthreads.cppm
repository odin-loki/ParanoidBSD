export module pbsd.port.wave2.hbsd.src.lib.libnetmap.nmctx_pthreads;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libnetmap/nmctx-pthreads.c
// void nmctx-pthreads_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libnetmap/nmctx-pthreads.c wave=wave2 loc=75
export namespace pbsd::port::wave2::hbsd::src::lib::libnetmap::nmctx_pthreads {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libnetmap::nmctx_pthreads
