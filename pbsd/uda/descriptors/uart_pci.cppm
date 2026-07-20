module;
#include <cstdint>

export module pbsd.uda.uart_pci;

export import pbsd.uda.schema;
export import pbsd.uda.ns8250;

/// PROVENANCE: hbsd/src/sys/dev/uart/uart_bus_pci.c, dev/ic/ns16550.h
export namespace pbsd::uda::uart_pci {

using namespace pbsd::uda::ns8250;

inline constexpr std::uint32_t kPciBarOffset = 0x10;
inline constexpr std::uint32_t kDefaultRclk    = 1843200;

inline constexpr std::uint16_t kPciVendorIntel     = 0x8086;
inline constexpr std::uint16_t kPciDevAmtSol         = 0x108F;
inline constexpr std::uint16_t kPciDevGeminiLakeUart = 0x31BC;

inline constexpr RegInsn kUartPciAttach[] = {
    {RegOp::Write32, kRegLcr, kLcrDlab | kLcr8bits, 0, 0},
    {RegOp::Write32, kRegDll, 0x01, 0, 0},
    {RegOp::Write32, kRegDlh, 0x00, 0, 0},
    {RegOp::Write32, kRegLcr, kLcr8bits, 0, 0},
    {RegOp::Write32, kRegFcr, kFcrEnable, 0, 0},
    {RegOp::Write32, kRegMcr, kMcrIe | kMcrDtr | kMcrRts, 0, 0},
    {RegOp::Write32, kRegIer, kIerErxrdy, 0, 0},
    {RegOp::CheckEq, kRegLsr, 0x60, 0x60, 1000},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kUartPciDetach[] = {
    {RegOp::Write32, kRegIer, 0x0, 0, 0},
    {RegOp::Write32, kRegMcr, 0x0, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor uart_pci_intel_sol() noexcept {
    return Descriptor{
        .name = "uart-pci-intel-amt-sol",
        .provenance = "hbsd/src/sys/dev/uart/uart_bus_pci.c",
        .device_class = DeviceClass::Input,
        .vendor_id = kPciVendorIntel,
        .device_id = kPciDevAmtSol,
        .init_sequence = kUartPciAttach,
        .reset_sequence = kUartPciDetach,
    };
}

[[nodiscard]] inline constexpr bool matches_pci(std::uint16_t vendor,
                                                std::uint16_t device) noexcept {
    return vendor == kPciVendorIntel
        && (device == kPciDevAmtSol || device == kPciDevGeminiLakeUart);
}

[[nodiscard]] inline constexpr int bar_rid() noexcept {
    return static_cast<int>(kPciBarOffset >> 4);
}

} // namespace pbsd::uda::uart_pci
