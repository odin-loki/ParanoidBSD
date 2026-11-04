module;
#include <cstdint>

export module pbsd.uda.pass;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/cam/scsi/scsi_pass.c
export namespace pbsd::uda::pass {

inline constexpr std::uint8_t kScsiOpTestUnitReady = 0x00;
inline constexpr std::uint8_t kScsiOpInquiry       = 0x12;
inline constexpr std::uint8_t kScsiOpReportLuns   = 0xA0;

inline constexpr RegInsn kPassInit[] = {
    {RegOp::Write8, 0x00, kScsiOpTestUnitReady, 0, 0},
    {RegOp::Write8, 0x01, kScsiOpInquiry, 0, 0},
    {RegOp::Write8, 0x02, kScsiOpReportLuns, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kPassReset[] = {
    {RegOp::Write8, 0x00, 0x00, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor pass_scsi_passthrough() noexcept {
    return Descriptor{
        .name = "pass-scsi-passthrough",
        .provenance = "hbsd/src/sys/cam/scsi/scsi_pass.c",
        .device_class = DeviceClass::Block,
        .vendor_id = 0x0000,
        .device_id = 0x0000,
        .init_sequence = kPassInit,
        .reset_sequence = kPassReset,
    };
}

} // namespace pbsd::uda::pass
