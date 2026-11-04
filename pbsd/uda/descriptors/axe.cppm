module;
#include <cstdint>

export module pbsd.uda.axe;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/usb/net/if_axereg.h
export namespace pbsd::uda::axe {

inline constexpr std::uint32_t kRegSwReset = 0x00;

inline constexpr std::uint8_t kCmdSwResetReg = 0x20;
inline constexpr std::uint8_t kSwResetClear  = 0x00;
inline constexpr std::uint8_t kSwResetRr     = 0x01;

inline constexpr RegInsn kAxeInit[] = {
    {RegOp::Write8, kRegSwReset, kCmdSwResetReg, 0, 0},
    {RegOp::Write8, kRegSwReset + 1, kSwResetRr, 0, 0},
    {RegOp::WaitUs, 0, 100, 0, 0},
    {RegOp::Write8, kRegSwReset + 1, kSwResetClear, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kAxeReset[] = {
    {RegOp::Write8, kRegSwReset + 1, kSwResetRr, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor axe_ax88772() noexcept {
    return Descriptor{
        .name = "axe-ax88772",
        .provenance = "hbsd/src/sys/dev/usb/net/if_axereg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x0B95,
        .device_id = 0x7720,
        .init_sequence = kAxeInit,
        .reset_sequence = kAxeReset,
    };
}

} // namespace pbsd::uda::axe
