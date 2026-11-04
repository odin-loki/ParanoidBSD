module;
#include <cstdint>

export module pbsd.uda.ufshci;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/ufshci/ufshci_reg.h
export namespace pbsd::uda::ufshci {

inline constexpr std::uint32_t kRegCap  = 0x00;
inline constexpr std::uint32_t kRegVer  = 0x08;
inline constexpr std::uint32_t kRegIs   = 0x20;
inline constexpr std::uint32_t kRegIe   = 0x24;
inline constexpr std::uint32_t kRegHcs  = 0x30;
inline constexpr std::uint32_t kRegHce  = 0x34;
inline constexpr std::uint32_t kRegUtrlba = 0x50;

inline constexpr std::uint32_t kHceEnable = 0x1;
inline constexpr std::uint32_t kHcsDp     = 0x1;

inline constexpr int kDeviceInitTimeoutMs = 2000;
inline constexpr int kSectorSize          = 512;
inline constexpr int kUtrEntries            = 32;

inline constexpr RegInsn kUfshciInit[] = {
    {RegOp::Write32, kRegIe, 0, 0, 0},
    {RegOp::Write32, kRegIs, 0xFFFFFFFFu, 0, 0},
    {RegOp::Write32, kRegHce, kHceEnable, 0, 0},
    {RegOp::WaitUs, 0, 1000, 0, 0},
    {RegOp::CheckEq, kRegHcs, kHcsDp, kHcsDp, kDeviceInitTimeoutMs * 1000},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kUfshciReset[] = {
    {RegOp::Write32, kRegHce, 0, 0, 0},
    {RegOp::Write32, kRegIs, 0xFFFFFFFFu, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor ufshci_generic() noexcept {
    return Descriptor{
        .name = "ufshci",
        .provenance = "hbsd/src/sys/dev/ufshci/ufshci_reg.h",
        .device_class = DeviceClass::Block,
        .vendor_id = 0,
        .device_id = 0,
        .init_sequence = kUfshciInit,
        .reset_sequence = kUfshciReset,
    };
}

[[nodiscard]] inline bool probe_mmio(SoftMmio& mem) noexcept {
    const auto cap = mem.read32(kRegCap);
    const auto ver = mem.read32(kRegVer);
    if (cap == 0xFFFFFFFFu || ver == 0xFFFFFFFFu) {
        return false;
    }
    mem.write32(kRegIs, 0xFFFFFFFFu);
    return mem.read32(kRegCap) != 0;
}

} // namespace pbsd::uda::ufshci
