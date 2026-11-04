module;
#include <cstdint>

export module pbsd.uda.clk;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/clk/clk.c
export namespace pbsd::uda::clk {

inline constexpr unsigned kMaxParents = 4;
inline constexpr unsigned kMaxName = 32;

[[nodiscard]] inline constexpr Descriptor clk_root() noexcept {
    return Descriptor{
        .name = "clk-root",
        .provenance = "hbsd/src/sys/dev/clk/clk.c",
        .device_class = DeviceClass::Sensor,
        .vendor_id = 0,
        .device_id = 0,
        .init_sequence = {},
        .reset_sequence = {},
    };
}

} // namespace pbsd::uda::clk
