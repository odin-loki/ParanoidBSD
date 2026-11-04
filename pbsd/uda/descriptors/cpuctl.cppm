module;

export module pbsd.uda.cpuctl;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/cpuctl/cpuctl.c
export namespace pbsd::uda::cpuctl {

inline constexpr unsigned kIoctlMsr = 0x80000001;
inline constexpr unsigned kIoctlCpuid = 0x80000002;

[[nodiscard]] inline constexpr Descriptor cpuctl_dev() noexcept {
    return Descriptor{
        .name = "cpuctl",
        .provenance = "hbsd/src/sys/dev/cpuctl/cpuctl.c",
        .device_class = DeviceClass::Sensor,
        .vendor_id = 0,
        .device_id = 0,
        .init_sequence = {},
        .reset_sequence = {},
    };
}

} // namespace pbsd::uda::cpuctl
