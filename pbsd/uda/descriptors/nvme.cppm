module;
#include <cstdint>

export module pbsd.uda.nvme;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/nvme/nvme.h — NVMe Base Spec register map.
export namespace pbsd::uda::nvme {

inline constexpr std::uint32_t kRegCap   = 0x00;
inline constexpr std::uint32_t kRegVs    = 0x08;
inline constexpr std::uint32_t kRegIntms = 0x0C;
inline constexpr std::uint32_t kRegIntmc = 0x10;
inline constexpr std::uint32_t kRegCc    = 0x14;
inline constexpr std::uint32_t kRegCsts  = 0x1C;
inline constexpr std::uint32_t kRegAqa   = 0x24;
inline constexpr std::uint32_t kRegAsq   = 0x28;
inline constexpr std::uint32_t kRegAcq   = 0x30;

inline constexpr std::uint32_t kCcEnMask  = 0x00000001;
inline constexpr std::uint32_t kCstsRdyMask = 0x00000001;

inline constexpr std::uint16_t kPciClassStorage = 0x0108;

inline constexpr RegInsn kNvmeInit[] = {
    {RegOp::Write32, kRegCc, 0x0, 0, 0},
    {RegOp::CheckEq, kRegCsts, 0x0, kCstsRdyMask, 1000},
    {RegOp::Write32, kRegIntms, 0xFFFFFFFF, 0, 0},
    {RegOp::Write32, kRegCc, kCcEnMask, 0, 0},
    {RegOp::CheckEq, kRegCsts, kCstsRdyMask, kCstsRdyMask, 1000},
    {RegOp::Write32, kRegIntmc, 0x1, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kNvmeReset[] = {
    {RegOp::Write32, kRegCc, 0x0, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor nvme_controller_1_4() noexcept {
    return Descriptor{
        .name = "nvme-controller-1.4",
        .provenance = "hbsd/src/sys/dev/nvme/nvme.h",
        .device_class = DeviceClass::Block,
        .vendor_id = 0xFFFF,
        .device_id = kPciClassStorage,
        .init_sequence = kNvmeInit,
        .reset_sequence = kNvmeReset,
    };
}

[[nodiscard]] inline constexpr bool matches_pci_class(std::uint16_t class_code) noexcept {
    return class_code == kPciClassStorage;
}

[[nodiscard]] inline bool probe_mmio(SoftMmio const& mem) noexcept {
    const auto cap_lo = mem.read32(kRegCap);
    const auto mqes = cap_lo & 0xFFFFu;
    return mqes >= 2;
}

} // namespace pbsd::uda::nvme
