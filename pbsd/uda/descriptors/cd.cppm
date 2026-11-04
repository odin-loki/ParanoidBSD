module;
#include <cstdint>

export module pbsd.uda.cd;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/cam/scsi/scsi_cd.c
export namespace pbsd::uda::cd {

inline constexpr std::uint8_t kScsiOpTestUnitReady = 0x00;
inline constexpr std::uint8_t kScsiOpInquiry       = 0x12;
inline constexpr std::uint8_t kScsiOpReadToc       = 0x43;
inline constexpr std::uint8_t kScsiOpRead10        = 0x28;

inline constexpr RegInsn kCdInit[] = {
    {RegOp::Write8, 0x00, kScsiOpTestUnitReady, 0, 0},
    {RegOp::Write8, 0x01, kScsiOpInquiry, 0, 0},
    {RegOp::Write8, 0x02, kScsiOpReadToc, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kCdReset[] = {
    {RegOp::Write8, 0x00, 0x00, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor cd_scsi_optical() noexcept {
    return Descriptor{
        .name = "cd-scsi-optical",
        .provenance = "hbsd/src/sys/cam/scsi/scsi_cd.c",
        .device_class = DeviceClass::Block,
        .vendor_id = 0x0000,
        .device_id = 0x0000,
        .init_sequence = kCdInit,
        .reset_sequence = kCdReset,
    };
}

} // namespace pbsd::uda::cd
