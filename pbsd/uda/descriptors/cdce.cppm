module;
#include <cstdint>

export module pbsd.uda.cdce;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/usb/net/if_cdce.c, USB CDC ECM spec.
export namespace pbsd::uda::cdce {

inline constexpr std::uint8_t kReqSetCommFeature = 0x02;
inline constexpr std::uint8_t kReqSetEthernetMulticastFilters = 0x40;
inline constexpr std::uint8_t kReqSetEthernetPacketFilter = 0x43;

inline constexpr std::uint8_t kPacketTypeDirected = 0x01;
inline constexpr std::uint8_t kPacketTypeBroadcast = 0x04;

inline constexpr RegInsn kCdceInit[] = {
    {RegOp::Write8, 0x00, kReqSetCommFeature, 0, 0},
    {RegOp::Write8, 0x01, 0x00, 0, 0},
    {RegOp::Write8, 0x02, kReqSetEthernetPacketFilter, 0, 0},
    {RegOp::Write8, 0x03, kPacketTypeDirected | kPacketTypeBroadcast, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kCdceReset[] = {
    {RegOp::Write8, 0x00, 0x00, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor cdce_ecm() noexcept {
    return Descriptor{
        .name = "cdce-ecm",
        .provenance = "hbsd/src/sys/dev/usb/net/if_cdce.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x0000,
        .device_id = 0x0000,
        .init_sequence = kCdceInit,
        .reset_sequence = kCdceReset,
    };
}

} // namespace pbsd::uda::cdce
