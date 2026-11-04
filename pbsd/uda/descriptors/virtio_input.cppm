module;
#include <cstdint>

export module pbsd.uda.virtio_input;

export import pbsd.uda.schema;
export import pbsd.uda.interp;
export import pbsd.uda.virtio.common;

/// PROVENANCE: hbsd/src/sys/dev/virtio/input/virtio_input.h
export namespace pbsd::uda::virtio_input {

inline constexpr std::uint32_t kCfgSelect = 0x00;
inline constexpr std::uint32_t kCfgSubsel = 0x01;
inline constexpr std::uint32_t kCfgSize   = 0x02;

inline constexpr std::uint8_t kSelCfgId     = 0x01;
inline constexpr std::uint8_t kSelEvBits    = 0x11;
inline constexpr std::uint8_t kSelAbsInfo   = 0x12;

inline constexpr RegInsn kInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor virtio_input_tablet() noexcept {
    return Descriptor{
        .name = "virtio-input-tablet",
        .provenance = "hbsd/src/sys/dev/virtio/input/",
        .device_class = DeviceClass::Input,
        .vendor_id = 0x1af4,
        .device_id = 0x1052,
        .init_sequence = kInit,
        .reset_sequence = kInit,
    };
}

} // namespace pbsd::uda::virtio_input
