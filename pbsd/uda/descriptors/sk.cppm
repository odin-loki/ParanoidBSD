module;
#include <cstdint>

export module pbsd.uda.sk;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/sk/if_skreg.h, if_sk.c
export namespace pbsd::uda::sk {

inline constexpr std::uint32_t kRegCsr = 0x0004;

inline constexpr std::uint32_t kCsrSwReset       = 0x0001;
inline constexpr std::uint32_t kCsrSwUnreset       = 0x0002;
inline constexpr std::uint32_t kCsrMasterReset     = 0x0004;
inline constexpr std::uint32_t kCsrMasterUnreset   = 0x0008;

inline constexpr std::uint16_t kPciVendorSk   = 0x1148;
inline constexpr std::uint16_t kPciDevYukon   = 0x4300;
inline constexpr std::uint16_t kPciDevYukon2  = 0x4320;

inline constexpr RegInsn kSkInit[] = {
    {RegOp::Write32, kRegCsr, kCsrMasterReset | kCsrSwReset, 0, 0},
    {RegOp::WaitUs, 0, 50, 0, 0},
    {RegOp::Write32, kRegCsr, kCsrMasterUnreset | kCsrSwUnreset, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kSkReset[] = {
    {RegOp::Write32, kRegCsr, kCsrMasterReset | kCsrSwReset, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor sk_yukon() noexcept {
    return Descriptor{
        .name = "sk-yukon",
        .provenance = "hbsd/src/sys/dev/sk/if_skreg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorSk,
        .device_id = kPciDevYukon,
        .init_sequence = kSkInit,
        .reset_sequence = kSkReset,
    };
}

[[nodiscard]] inline constexpr Descriptor sk_yukon2() noexcept {
    return Descriptor{
        .name = "sk-yukon2",
        .provenance = "hbsd/src/sys/dev/sk/if_skreg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorSk,
        .device_id = kPciDevYukon2,
        .init_sequence = kSkInit,
        .reset_sequence = kSkReset,
    };
}

[[nodiscard]] inline constexpr bool matches_pci(std::uint16_t vendor,
                                                std::uint16_t device) noexcept {
    return vendor == kPciVendorSk
        && (device == kPciDevYukon || device == kPciDevYukon2);
}

[[nodiscard]] inline bool probe_mmio(SoftMmio& mem) noexcept {
    mem.write32(kRegCsr, kCsrMasterReset | kCsrSwReset);
    const auto csr = mem.read32(kRegCsr);
    return csr != 0xFFFFFFFFu;
}

} // namespace pbsd::uda::sk
