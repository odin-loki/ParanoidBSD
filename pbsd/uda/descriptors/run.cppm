module;
#include <cstdint>

export module pbsd.uda.run;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/run/if_runreg.h
export namespace pbsd::uda::run {

inline constexpr std::uint8_t kRegMacCtrl = 0x0500;
inline constexpr std::uint8_t kRegBbCtrl  = 0x0501;
inline constexpr std::uint8_t kMacCtrlRfOn = 0x04;

inline constexpr RegInsn kRunInit[] = {
    {RegOp::Write8, kRegBbCtrl, 0x00, 0, 0},
    {RegOp::WaitUs, 0, 50, 0, 0},
    {RegOp::Write8, kRegMacCtrl, kMacCtrlRfOn, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kRunReset[] = {
    {RegOp::Write8, kRegMacCtrl, 0, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor run_rt2870() noexcept {
    return Descriptor{
        .name = "run-rt2870",
        .provenance = "hbsd/src/sys/dev/run/if_runreg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x148F,
        .device_id = 0x2870,
        .init_sequence = kRunInit,
        .reset_sequence = kRunReset,
    };
}

} // namespace pbsd::uda::run
