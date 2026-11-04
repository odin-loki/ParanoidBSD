module;
#include <cstdint>

export module pbsd.uda.xl;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/xl/if_xlreg.h, if_xl.c
export namespace pbsd::uda::xl {

inline constexpr std::uint32_t kRegCommand = 0x0E;

inline constexpr std::uint32_t kCmdReset     = 0x0000;
inline constexpr std::uint32_t kCmdRxEnable  = 0x2000;
inline constexpr std::uint32_t kCmdTxEnable  = 0x4800;

inline constexpr std::uint16_t kPciVendor3Com = 0x10B7;
inline constexpr std::uint16_t kPciDev905cTx  = 0x9200;

inline constexpr RegInsn kXlInit[] = {
    {RegOp::Write32, kRegCommand, kCmdReset, 0, 0},
    {RegOp::WaitUs, 0, 50, 0, 0},
    {RegOp::Write32, kRegCommand, kCmdRxEnable, 0, 0},
    {RegOp::Write32, kRegCommand, kCmdTxEnable, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kXlReset[] = {
    {RegOp::Write32, kRegCommand, kCmdReset, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor xl_3c905c_tx() noexcept {
    return Descriptor{
        .name = "xl-3c905c-tx",
        .provenance = "hbsd/src/sys/dev/xl/if_xlreg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendor3Com,
        .device_id = kPciDev905cTx,
        .init_sequence = kXlInit,
        .reset_sequence = kXlReset,
    };
}

[[nodiscard]] inline constexpr bool matches_pci(std::uint16_t vendor,
                                                std::uint16_t device) noexcept {
    return vendor == kPciVendor3Com && device == kPciDev905cTx;
}

[[nodiscard]] inline bool probe_mmio(SoftMmio& mem) noexcept {
    mem.write32(kRegCommand, kCmdReset);
    const auto status = mem.read32(kRegCommand);
    return status != 0xFFFFFFFFu;
}

} // namespace pbsd::uda::xl
