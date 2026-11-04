module;
#include <cstdint>

export module pbsd.uda.mpt_cam;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;
export import pbsd.uda.cam.xpt;

/// PROVENANCE: hbsd/src/sys/dev/mpt/{mpt_reg.h,mpt_cam.c} — MPI doorbell CAM path.
export namespace pbsd::uda::mpt_cam {

inline constexpr std::uint32_t kOffsetDoorbell     = 0x00;
inline constexpr std::uint32_t kOffsetIntrStatus   = 0x30;
inline constexpr std::uint32_t kOffsetRequestQ       = 0x40;
inline constexpr std::uint32_t kOffsetReplyQ         = 0x44;

inline constexpr std::uint32_t kDbStateReady       = 0x10000000;
inline constexpr std::uint32_t kDbStateRunning     = 0x20000000;
inline constexpr std::uint32_t kDbInitHost         = 0x04;
inline constexpr std::uint32_t kIntrReplyReady     = 0x00000008;

inline constexpr RegInsn kMptCamInit[] = {
    {RegOp::Write32, kOffsetDoorbell, kDbInitHost, 0, 0},
    {RegOp::CheckEq, kOffsetDoorbell, kDbStateReady, 0xF0000000, 5000},
    {RegOp::Write32, kOffsetDoorbell, kDbStateRunning, 0, 0},
    {RegOp::CheckEq, kOffsetIntrStatus, kIntrReplyReady, kIntrReplyReady, 1000},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kMptCamReset[] = {
    {RegOp::Write32, kOffsetDoorbell, 0x40000000, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor mpt_lsi_cam() noexcept {
    return Descriptor{
        .name = "mpt-lsi-cam",
        .provenance = "hbsd/src/sys/dev/mpt/mpt_reg.h",
        .device_class = DeviceClass::Block,
        .vendor_id = 0x1000,
        .device_id = 0x0030,
        .init_sequence = kMptCamInit,
        .reset_sequence = kMptCamReset,
    };
}

[[nodiscard]] inline bool probe_doorbell(SoftMmio const& mem) noexcept {
    const auto db = mem.read32(kOffsetDoorbell);
    return (db & 0xF0000000u) == kDbStateReady || (db & 0xF0000000u) == kDbStateRunning;
}

[[nodiscard]] constexpr bool matches_pci(std::uint16_t vendor, std::uint16_t device) noexcept {
    return vendor == 0x1000 && device == 0x0030;
}

} // namespace pbsd::uda::mpt_cam
