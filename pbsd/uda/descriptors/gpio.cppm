module;
#include <cstdint>

export module pbsd.uda.gpio;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/gpio/chvgpio_reg.h
export namespace pbsd::uda::gpio {

inline constexpr std::uint32_t kRegInterruptStatus = 0x0300;
inline constexpr std::uint32_t kRegInterruptMask    = 0x0380;
inline constexpr std::uint32_t kRegPadCfg0          = 0x4400;
inline constexpr std::uint32_t kRegPadCfg1          = 0x4404;

inline constexpr std::uint32_t kPadCfg0GpiocfgGpi = 0x00000200;
inline constexpr std::uint32_t kPadCfg0GpiocfgGpo = 0x00000100;
inline constexpr std::uint32_t kPadCfg1IntwakeRising = 0x00000002;

inline constexpr RegInsn kGpioInit[] = {
    {RegOp::Write32, kRegInterruptMask, 0x0, 0, 0},
    {RegOp::Write32, kRegPadCfg0, kPadCfg0GpiocfgGpi, 0, 0},
    {RegOp::Write32, kRegPadCfg1, kPadCfg1IntwakeRising, 0, 0},
    {RegOp::Write32, kRegInterruptStatus, 0xFFFFFFFFu, 0, 0},
    {RegOp::Write32, kRegPadCfg0 + 4, kPadCfg0GpiocfgGpo, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kGpioReset[] = {
    {RegOp::Write32, kRegInterruptMask, 0xFFFFFFFFu, 0, 0},
    {RegOp::Write32, kRegPadCfg0, 0x0, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor chv_gpio_1_0() noexcept {
    return Descriptor{
        .name = "chv-gpio-1.0",
        .provenance = "hbsd/src/sys/dev/gpio/chvgpio_reg.h",
        .device_class = DeviceClass::Sensor,
        .vendor_id = 0x8086,
        .device_id = 0x0000,
        .init_sequence = kGpioInit,
        .reset_sequence = kGpioReset,
    };
}

[[nodiscard]] inline bool probe_mmio(SoftMmio const& mem) noexcept {
    return mem.read32(kRegPadCfg0) != 0xFFFFFFFFu;
}

} // namespace pbsd::uda::gpio
