module;
#include <cstdint>

export module pbsd.uda.led;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/dev/led/led.c
export namespace pbsd::uda::led {

inline constexpr unsigned kMaxNameLen = 32;

enum class State : unsigned {
    Off = 0,
    On  = 1,
    Blink,
};

struct SoftState {
    char     name[kMaxNameLen]{};
    State    state{State::Off};
    int      unit{};
    bool     registered{};
};

[[nodiscard]] inline Status register_led(SoftState& sc, int unit) noexcept {
    if (sc.registered) {
        return Status::Busy;
    }
    if (unit < 0) {
        return Status::Invalid;
    }
    sc.unit = unit;
    sc.registered = true;
    return Status::Ok;
}

[[nodiscard]] inline Status set_state(SoftState& sc, State state) noexcept {
    if (!sc.registered) {
        return Status::Invalid;
    }
    sc.state = state;
    return Status::Ok;
}

inline constexpr RegInsn kLedInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor led_generic() noexcept {
    return Descriptor{
        .name = "led",
        .provenance = "hbsd/src/sys/dev/led/led.c",
        .device_class = DeviceClass::Display,
        .vendor_id = 0,
        .device_id = 0,
        .init_sequence = kLedInit,
        .reset_sequence = kLedInit,
    };
}

} // namespace pbsd::uda::led
