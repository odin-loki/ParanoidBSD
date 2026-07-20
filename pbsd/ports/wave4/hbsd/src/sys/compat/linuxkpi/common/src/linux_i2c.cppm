export module pbsd.port.wave4.hbsd.src.sys.compat.linuxkpi.common.src.linux_i2c;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/compat/linuxkpi/common/src/linux_i2c.c
// void linux_i2c_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/compat/linuxkpi/common/src/linux_i2c.c wave=wave4 loc=381
export namespace pbsd::port::wave4::hbsd::src::sys::compat::linuxkpi::common::src::linux_i2c {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::compat::linuxkpi::common::src::linux_i2c
