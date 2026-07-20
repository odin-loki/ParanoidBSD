module;
#include <cstdint>

export module pbsd.uda.rtwn;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/rtwn/if_rtwnreg.h
export namespace pbsd::uda::rtwn {

inline constexpr std::uint32_t kRegSysFuncEn = 0x0102;
inline constexpr std::uint32_t kRegCr        = 0x0100;
inline constexpr std::uint32_t kCrHciTxen    = 0x00000004;

inline constexpr RegInsn kRtwnInit[] = {
    {RegOp::Write32, kRegSysFuncEn, 0x03, 0, 0},
    {RegOp::WaitUs, 0, 50, 0, 0},
    {RegOp::Write32, kRegCr, kCrHciTxen, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kRtwnReset[] = {
    {RegOp::Write32, kRegCr, 0, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor rtwn_rtl8192ce() noexcept {
    return Descriptor{
        .name = "rtwn-rtl8192ce",
        .provenance = "hbsd/src/sys/dev/rtwn/if_rtwnreg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x10EC,
        .device_id = 0x8176,
        .init_sequence = kRtwnInit,
        .reset_sequence = kRtwnReset,
    };
}

} // namespace pbsd::uda::rtwn
