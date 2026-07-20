export module pbsd.port.wave5.hbsd.src.sys.dev.igc.if_igc;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/igc/if_igc.c
// void if_igc_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/igc/if_igc.c wave=wave5 loc=3310
export namespace pbsd::port::wave5::hbsd::src::sys::dev::igc::if_igc {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::igc::if_igc
