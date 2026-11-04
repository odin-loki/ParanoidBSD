module;
#include <cstdint>

export module pbsd.uda.ses;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/cam/scsi/scsi_ses.c, SES spec
export namespace pbsd::uda::ses {

inline constexpr std::uint8_t kScsiOpInquiry     = 0x12;
inline constexpr std::uint8_t kScsiOpReceiveDiag = 0x1C;
inline constexpr std::uint8_t kScsiOpSendDiag    = 0x1D;
inline constexpr std::uint8_t kSesPageSupported  = 0x00;

inline constexpr RegInsn kSesInit[] = {
    {RegOp::Write8, 0x00, kScsiOpInquiry, 0, 0},
    {RegOp::Write8, 0x01, kScsiOpReceiveDiag, 0, 0},
    {RegOp::Write8, 0x02, kSesPageSupported, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kSesReset[] = {
    {RegOp::Write8, 0x00, kScsiOpSendDiag, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor ses_enclosure() noexcept {
    return Descriptor{
        .name = "ses-enclosure",
        .provenance = "hbsd/src/sys/cam/scsi/scsi_ses.c",
        .device_class = DeviceClass::Sensor,
        .vendor_id = 0x0000,
        .device_id = 0x0000,
        .init_sequence = kSesInit,
        .reset_sequence = kSesReset,
    };
}

} // namespace pbsd::uda::ses
