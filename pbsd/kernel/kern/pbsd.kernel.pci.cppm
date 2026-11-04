module;
#include <cstdint>

export module pbsd.kernel.pci;

export import pbsd.core;

/// Wave 4/5 — PCI config space offsets from sys/pciio.h / pci_regs.h.
export namespace pbsd::kernel::pci {

inline constexpr std::uint8_t kCfgVendorId  = 0x00;
inline constexpr std::uint8_t kCfgDeviceId  = 0x02;
inline constexpr std::uint8_t kCfgCommand   = 0x04;
inline constexpr std::uint8_t kCfgStatus    = 0x06;
inline constexpr std::uint8_t kCfgRevId     = 0x08;
inline constexpr std::uint8_t kCfgClass     = 0x09;
inline constexpr std::uint8_t kCfgBar0      = 0x10;
inline constexpr std::uint8_t kCfgBar1      = 0x14;
inline constexpr std::uint8_t kCfgSubVendor = 0x2c;
inline constexpr std::uint8_t kCfgSubDevice = 0x2e;

inline constexpr std::uint16_t kCmdIoEnable  = 0x0001;
inline constexpr std::uint16_t kCmdMemEnable = 0x0002;
inline constexpr std::uint16_t kCmdBusMaster = 0x0004;

[[nodiscard]] constexpr Status validate_bar_index(unsigned idx) noexcept {
    if (idx > 5) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::pci
