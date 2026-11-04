module;
#include <cstdint>

export module pbsd.uda.virtio.common;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/virtio/{virtio_config.h,virtio_ids.h,virtio_mmio.h,
/// virtio_pci_legacy_var.h} — public OASIS / BSD-licensed headers only.
export namespace pbsd::uda::virtio {

inline constexpr std::uint32_t kMmioMagicValue     = 0x000;
inline constexpr std::uint32_t kMmioVersion        = 0x004;
inline constexpr std::uint32_t kMmioDeviceId       = 0x008;
inline constexpr std::uint32_t kMmioVendorId       = 0x00c;
inline constexpr std::uint32_t kMmioHostFeatures     = 0x010;
inline constexpr std::uint32_t kMmioGuestFeatures    = 0x020;
inline constexpr std::uint32_t kMmioQueueSel         = 0x030;
inline constexpr std::uint32_t kMmioQueueReady       = 0x044;
inline constexpr std::uint32_t kMmioQueueNotify      = 0x050;
inline constexpr std::uint32_t kMmioInterruptStatus  = 0x060;
inline constexpr std::uint32_t kMmioInterruptAck     = 0x064;
inline constexpr std::uint32_t kMmioStatus           = 0x070;
inline constexpr std::uint32_t kMmioConfig           = 0x100;

inline constexpr std::uint32_t kMmioMagicVirt        = 0x74726976; // "virt"

inline constexpr std::uint8_t kStatusReset      = 0x00;
inline constexpr std::uint8_t kStatusAck        = 0x01;
inline constexpr std::uint8_t kStatusDriver     = 0x02;
inline constexpr std::uint8_t kStatusDriverOk   = 0x04;
inline constexpr std::uint8_t kFeaturesOk       = 0x08;
inline constexpr std::uint8_t kStatusNeedsReset = 0x40;
inline constexpr std::uint8_t kStatusFailed     = 0x80;

inline constexpr std::uint16_t kPciVendorRedHat = 0x1AF4;
inline constexpr std::uint16_t kPciDevNet       = 0x1000;
inline constexpr std::uint16_t kPciDevBlk       = 0x1001;
inline constexpr std::uint16_t kPciDevBalloon   = 0x1002;
inline constexpr std::uint16_t kPciDevScsi      = 0x1004;
inline constexpr std::uint16_t kPciDevConsole   = 0x1003;
inline constexpr std::uint16_t kPciDevEntropy   = 0x1005;
inline constexpr std::uint16_t kPciDevGpu       = 0x1050;

inline constexpr std::uint32_t kIdNetwork = 1;
inline constexpr std::uint32_t kIdBlock   = 2;
inline constexpr std::uint32_t kIdConsole = 3;
inline constexpr std::uint32_t kIdEntropy = 4;
inline constexpr std::uint32_t kIdBalloon = 5;
inline constexpr std::uint32_t kIdScsi    = 8;
inline constexpr std::uint32_t kIdGpu     = 16;

inline constexpr RegInsn kMmioAckDriver[] = {
    {RegOp::Write32, kMmioStatus, kStatusAck, 0, 0},
    {RegOp::Write32, kMmioStatus, kStatusAck | kStatusDriver, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kMmioFeaturesOk[] = {
    {RegOp::Write32, kMmioStatus,
     kStatusAck | kStatusDriver | kFeaturesOk, 0, 0},
    {RegOp::CheckEq, kMmioStatus, kFeaturesOk, kFeaturesOk, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kMmioDriverOk[] = {
    {RegOp::Write32, kMmioQueueSel, 0, 0, 0},
    {RegOp::Write32, kMmioQueueReady, 0x1, 0, 0},
    {RegOp::Write32, kMmioStatus,
     kStatusAck | kStatusDriver | kFeaturesOk | kStatusDriverOk, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kMmioReset[] = {
    {RegOp::Write32, kMmioStatus, kStatusReset, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr bool matches_pci(std::uint16_t vendor,
                                                std::uint16_t device) noexcept {
    return vendor == kPciVendorRedHat;
}

[[nodiscard]] inline constexpr bool probe_mmio_device(SoftMmio const& mem,
                                                      std::uint32_t expect_id) noexcept {
    if (mem.read32(kMmioMagicValue) != kMmioMagicVirt) {
        return false;
    }
    return mem.read32(kMmioDeviceId) == expect_id;
}

} // namespace pbsd::uda::virtio
