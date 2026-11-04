module;
#include <cstdint>

export module pbsd.uda.pci_hostbridge;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/pci/pci_host_generic.c — PCI host bridge probe.
export namespace pbsd::uda::pci_hostbridge {

inline constexpr std::uint32_t kCfgVendor = 0x00;
inline constexpr std::uint32_t kCfgDevice = 0x02;
inline constexpr std::uint32_t kCfgClass  = 0x0B;
inline constexpr std::uint32_t kCfgHeader = 0x0E;

inline constexpr std::uint16_t kClassHostBridge = 0x0600;

inline constexpr RegInsn kPciHostInit[] = {
    {RegOp::Read32, kCfgVendor, 0, 0, 0},
    {RegOp::Read32, kCfgDevice, 0, 0, 0},
    {RegOp::Read32, kCfgClass, 0, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kPciHostReset[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor pci_hostbridge_generic() noexcept {
    return Descriptor{
        .name = "pci-hostbridge-generic",
        .provenance = "hbsd/src/sys/dev/pci/pci_host_generic.c",
        .device_class = DeviceClass::Sensor,
        .vendor_id = 0xFFFF,
        .device_id = 0xFFFF,
        .init_sequence = kPciHostInit,
        .reset_sequence = kPciHostReset,
    };
}

[[nodiscard]] inline bool probe_mmio(SoftMmio const& mem) noexcept {
    const auto cls = mem.read32(kCfgClass) & 0xFFFF;
    const auto hdr = mem.read32(kCfgHeader) & 0xFF;
    return cls == kClassHostBridge && hdr == 0;
}

} // namespace pbsd::uda::pci_hostbridge
