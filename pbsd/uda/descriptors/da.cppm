module;
#include <cstdint>

export module pbsd.uda.da;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/cam/scsi/scsi_da.c (disk peripheral tags).
export namespace pbsd::uda::da {

inline constexpr std::uint32_t kMaxLba48Bits = 48;
inline constexpr std::uint32_t kSector512 = 512;
inline constexpr std::uint32_t kSector4k = 4096;

inline constexpr RegInsn kDaInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor da_direct() noexcept {
    return Descriptor{
        .name = "cam-da",
        .provenance = "hbsd/src/sys/cam/scsi/scsi_da.c",
        .device_class = DeviceClass::Block,
        .vendor_id = 0,
        .device_id = 0,
        .init_sequence = kDaInit,
        .reset_sequence = kDaInit,
    };
}

[[nodiscard]] constexpr bool valid_sector(std::uint32_t n) noexcept {
    return n == kSector512 || n == kSector4k;
}

} // namespace pbsd::uda::da
