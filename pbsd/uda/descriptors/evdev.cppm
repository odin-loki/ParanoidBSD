module;

export module pbsd.uda.evdev;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/evdev/evdev.c
export namespace pbsd::uda::evdev {

inline constexpr unsigned kMaxSlots = 10;
inline constexpr unsigned kBufSize = 64;

enum class EventType : unsigned short { Syn = 0, Key = 1, Rel = 2, Abs = 3 };

[[nodiscard]] inline constexpr Descriptor evdev_generic() noexcept {
    return Descriptor{
        .name = "evdev",
        .provenance = "hbsd/src/sys/dev/evdev/evdev.c",
        .device_class = DeviceClass::Input,
        .vendor_id = 0,
        .device_id = 0,
        .init_sequence = {},
        .reset_sequence = {},
    };
}

} // namespace pbsd::uda::evdev
