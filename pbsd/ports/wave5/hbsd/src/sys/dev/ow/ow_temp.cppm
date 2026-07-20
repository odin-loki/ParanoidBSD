export module pbsd.port.wave5.hbsd.src.sys.dev.ow.ow_temp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/ow/ow_temp.c
// void ow_temp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/ow/ow_temp.c wave=wave5 loc=279
export namespace pbsd::port::wave5::hbsd::src::sys::dev::ow::ow_temp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::ow::ow_temp
