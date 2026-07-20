export module pbsd.port.wave2.hbsd.src.lib.libhbsdcontrol.disallow_map32bit;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libhbsdcontrol/disallow_map32bit.c
// void disallow_map32bit_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libhbsdcontrol/disallow_map32bit.c wave=wave2 loc=315
export namespace pbsd::port::wave2::hbsd::src::lib::libhbsdcontrol::disallow_map32bit {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libhbsdcontrol::disallow_map32bit
