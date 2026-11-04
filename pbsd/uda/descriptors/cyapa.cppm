module;
#include <cstdint>

export module pbsd.uda.cyapa;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/cyapa/cyapa.c
export namespace pbsd::uda::cyapa {

inline constexpr std::uint8_t kRegDevStatus = 0x00;
inline constexpr std::uint8_t kStatusOperational = 0x03;
inline constexpr std::uint16_t kI2cAddr = 0x67;

inline constexpr RegInsn kCyapaInit[] = {
    {RegOp::Write8, kRegDevStatus, kStatusOperational, 0, 0},
    {RegOp::WaitUs, 0, 10, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor cyapa_gen3() noexcept {
    return Descriptor{
        .name = "cyapa-gen3",
        .provenance = "hbsd/src/sys/dev/cyapa/cyapa.c",
        .device_class = DeviceClass::Input,
        .vendor_id = 0,
        .device_id = kI2cAddr,
        .init_sequence = kCyapaInit,
        .reset_sequence = kCyapaInit,
    };
}

} // namespace pbsd::uda::cyapa
