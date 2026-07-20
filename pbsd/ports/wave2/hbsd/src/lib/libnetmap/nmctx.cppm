export module pbsd.port.wave2.hbsd.src.lib.libnetmap.nmctx;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libnetmap/nmctx.c
// void nmctx_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libnetmap/nmctx.c wave=wave2 loc=140
export namespace pbsd::port::wave2::hbsd::src::lib::libnetmap::nmctx {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libnetmap::nmctx
