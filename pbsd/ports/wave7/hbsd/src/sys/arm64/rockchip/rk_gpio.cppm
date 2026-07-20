export module pbsd.port.wave7.hbsd.src.sys.arm64.rockchip.rk_gpio;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/arm64/rockchip/rk_gpio.c
// void rk_gpio_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/arm64/rockchip/rk_gpio.c wave=wave7 loc=989
export namespace pbsd::port::wave7::hbsd::src::sys::arm64::rockchip::rk_gpio {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::arm64::rockchip::rk_gpio
