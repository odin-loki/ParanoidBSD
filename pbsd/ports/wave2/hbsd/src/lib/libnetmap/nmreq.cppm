export module pbsd.port.wave2.hbsd.src.lib.libnetmap.nmreq;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libnetmap/nmreq.c
// void nmreq_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libnetmap/nmreq.c wave=wave2 loc=716
export namespace pbsd::port::wave2::hbsd::src::lib::libnetmap::nmreq {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libnetmap::nmreq
