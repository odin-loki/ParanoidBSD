module;
#include <cstdint>

export module pbsd.uda.backlight;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/sys/backlight.h, hbsd/src/sys/dev/backlight/backlight.c
export namespace pbsd::uda::backlight {

inline constexpr unsigned kMaxLevels      = 100;
inline constexpr unsigned kMaxNameLength   = 64;

enum class InfoType : unsigned {
    Panel    = 0,
    Keyboard = 1,
};

struct Props {
    std::uint32_t brightness{};
    std::uint32_t nlevels{};
    std::uint32_t levels[kMaxLevels]{};
};

struct Info {
    char name[kMaxNameLength]{};
    InfoType type{InfoType::Panel};
};

struct SoftState {
    int           unit{};
    std::uint32_t cached_brightness{};
    bool          registered{};
};

[[nodiscard]] inline Status validate_props(const Props& props) noexcept {
    if (props.nlevels == 0 || props.nlevels > kMaxLevels) {
        return Status::Invalid;
    }
    if (props.brightness >= props.nlevels) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status register_device(SoftState& sc, std::uint32_t brightness) noexcept {
    if (sc.registered) {
        return Status::Busy;
    }
    sc.cached_brightness = brightness;
    sc.registered = true;
    return Status::Ok;
}

[[nodiscard]] inline Status update_status(SoftState& sc, const Props& props) noexcept {
    if (!sc.registered || validate_props(props) != Status::Ok) {
        return Status::Invalid;
    }
    if (props.brightness == sc.cached_brightness) {
        return Status::Ok;
    }
    sc.cached_brightness = props.brightness;
    return Status::Ok;
}

inline constexpr RegInsn kBacklightInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor backlight_generic() noexcept {
    return Descriptor{
        .name = "backlight",
        .provenance = "hbsd/src/sys/dev/backlight/backlight.c",
        .device_class = DeviceClass::Display,
        .vendor_id = 0,
        .device_id = 0,
        .init_sequence = kBacklightInit,
        .reset_sequence = kBacklightInit,
    };
}

} // namespace pbsd::uda::backlight
