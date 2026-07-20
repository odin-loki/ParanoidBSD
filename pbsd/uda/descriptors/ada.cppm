module;
#include <cstdint>

export module pbsd.uda.ada;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/cam/ata/ata_all.h, cam/ata/ata_da.c
export namespace pbsd::uda::ada {

inline constexpr std::uint8_t kAtaCmdIdentify = 0xEC;
inline constexpr std::uint8_t kAtaCmdReadDma   = 0xC8;
inline constexpr std::uint8_t kAtaCmdWriteDma  = 0xCA;
inline constexpr std::uint8_t kAtaCmdFlush     = 0xE7;

inline constexpr RegInsn kAdaInit[] = {
    {RegOp::Write8, 0x00, kAtaCmdIdentify, 0, 0},
    {RegOp::WaitUs, 0, 10, 0, 0},
    {RegOp::Write8, 0x01, kAtaCmdFlush, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kAdaReset[] = {
    {RegOp::Write8, 0x00, 0x00, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor ada_ata_direct() noexcept {
    return Descriptor{
        .name = "ada-ata-direct",
        .provenance = "hbsd/src/sys/cam/ata/ata_da.c",
        .device_class = DeviceClass::Block,
        .vendor_id = 0x0000,
        .device_id = 0x0000,
        .init_sequence = kAdaInit,
        .reset_sequence = kAdaReset,
    };
}

} // namespace pbsd::uda::ada
