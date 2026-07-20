module;

#include <cstdint>



export module pbsd.uda.virtio_scsi_cam;

import pbsd.core;


export import pbsd.uda.schema;

export import pbsd.uda.virtio_scsi;



/// PROVENANCE: hbsd/src/sys/dev/virtio/scsi — CAM periph attach path stub.

export namespace pbsd::uda::virtio_scsi_cam {



inline constexpr std::uint32_t kVscCamMmioBase = 0x0;

inline constexpr std::uint32_t kVscCamStatus   = 0x00;

inline constexpr std::uint32_t kVscCamControl  = 0x04;



inline constexpr std::uint32_t kVscCamEnable   = 0x00000001;

inline constexpr std::uint32_t kVscCamReady     = 0x00000002;



inline constexpr RegInsn kVirtioScsiCamInit[] = {

    {RegOp::Write32, kVscCamMmioBase + kVscCamControl, kVscCamEnable, 0, 0},

    {RegOp::CheckEq, kVscCamMmioBase + kVscCamStatus, kVscCamReady, 0xFFFFFFFFu, 500},

    {RegOp::Done, 0, 0, 0, 0},

};



inline constexpr RegInsn kVirtioScsiCamStop[] = {

    {RegOp::Write32, kVscCamMmioBase + kVscCamControl, 0x0, 0, 0},

    {RegOp::Done, 0, 0, 0, 0},

};



[[nodiscard]] inline constexpr Descriptor virtio_scsi_cam() noexcept {

    return Descriptor{

        .name = "virtio-scsi-cam",

        .provenance = "hbsd/src/sys/dev/virtio/scsi/virtio_scsi.c",

        .device_class = DeviceClass::Block,

        .vendor_id = 0x1AF4,

        .device_id = 0x1048,

        .init_sequence = kVirtioScsiCamInit,

        .reset_sequence = kVirtioScsiCamStop,

    };

}



[[nodiscard]] inline constexpr bool matches_virtio_scsi(std::uint16_t device_id) noexcept {

    return device_id == 0x1048 || device_id == 0x1049;

}



} // namespace pbsd::uda::virtio_scsi_cam

