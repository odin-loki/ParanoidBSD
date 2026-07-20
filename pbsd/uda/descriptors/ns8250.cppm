module;
#include <cstdint>

export module pbsd.uda.ns8250;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/ic/ns16550.h, uart/uart_dev_ns8250.c
export namespace pbsd::uda::ns8250 {

inline constexpr std::uint32_t kRegData = 0;
inline constexpr std::uint32_t kRegIer  = 1;
inline constexpr std::uint32_t kRegLcr  = 3;
inline constexpr std::uint32_t kRegMcr  = 4;
inline constexpr std::uint32_t kRegLsr  = 5;
inline constexpr std::uint32_t kRegDll  = 0;
inline constexpr std::uint32_t kRegDlh  = 1;
inline constexpr std::uint32_t kRegFcr  = 2;

inline constexpr std::uint32_t kLcrDlab   = 0x80;
inline constexpr std::uint32_t kLcr8bits  = 0x03;
inline constexpr std::uint32_t kMcrIe     = 0x08;
inline constexpr std::uint32_t kMcrDtr    = 0x01;
inline constexpr std::uint32_t kMcrRts    = 0x02;
inline constexpr std::uint32_t kFcrEnable = 0x01;
inline constexpr std::uint32_t kIerErxrdy = 0x01;

inline constexpr RegInsn kNs8250Init[] = {
    {RegOp::Write32, kRegLcr, kLcrDlab | kLcr8bits, 0, 0},
    {RegOp::Write32, kRegDll, 0x01, 0, 0},
    {RegOp::Write32, kRegDlh, 0x00, 0, 0},
    {RegOp::Write32, kRegLcr, kLcr8bits, 0, 0},
    {RegOp::Write32, kRegFcr, kFcrEnable, 0, 0},
    {RegOp::Write32, kRegMcr, kMcrIe | kMcrDtr | kMcrRts, 0, 0},
    {RegOp::CheckEq, kRegLsr, 0x60, 0x60, 1000},
    {RegOp::Write32, kRegIer, kIerErxrdy, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kNs8250Reset[] = {
    {RegOp::Write32, kRegIer, 0x0, 0, 0},
    {RegOp::Write32, kRegMcr, 0x0, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor ns8250_com1() noexcept {
    return Descriptor{
        .name = "ns8250-com1",
        .provenance = "hbsd/src/sys/dev/ic/ns16550.h",
        .device_class = DeviceClass::Input,
        .vendor_id = 0x0000,
        .device_id = 0x0000,
        .init_sequence = kNs8250Init,
        .reset_sequence = kNs8250Reset,
    };
}

[[nodiscard]] inline bool probe_mmio(SoftMmio const& mem) noexcept {
    return mem.read32(kRegLsr) != 0xFFu;
}

} // namespace pbsd::uda::ns8250
