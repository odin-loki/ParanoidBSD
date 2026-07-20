module;

export module pbsd.uda.chromebook_platform;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/chromebook_platform/chromebook_platform.c
export namespace pbsd::uda::chromebook_platform {

inline constexpr unsigned kMaxButtons = 4;

[[nodiscard]] inline constexpr Descriptor cros_platform() noexcept {
    return Descriptor{
        .name = "chromebook-platform",
        .provenance = "hbsd/src/sys/dev/chromebook_platform/chromebook_platform.c",
        .device_class = DeviceClass::Input,
        .vendor_id = 0,
        .device_id = 0,
        .init_sequence = {},
        .reset_sequence = {},
    };
}

} // namespace pbsd::uda::chromebook_platform
