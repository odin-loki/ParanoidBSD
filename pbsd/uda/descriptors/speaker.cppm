module;
#include <cstdint>

export module pbsd.uda.speaker;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/dev/speaker/speaker.c
export namespace pbsd::uda::speaker {

enum class Tone : unsigned {
    Off = 0,
    Low,
    High,
};

struct SoftState {
    Tone  tone{Tone::Off};
    bool  attached{};
};

[[nodiscard]] inline Status attach(SoftState& sc) noexcept {
    if (sc.attached) {
        return Status::Busy;
    }
    sc.tone = Tone::Off;
    sc.attached = true;
    return Status::Ok;
}

[[nodiscard]] inline Status beep(SoftState& sc, Tone tone) noexcept {
    if (!sc.attached) {
        return Status::Invalid;
    }
    sc.tone = tone;
    return Status::Ok;
}

inline constexpr RegInsn kSpeakerInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor speaker_dev() noexcept {
    return Descriptor{
        .name = "speaker",
        .provenance = "hbsd/src/sys/dev/speaker/speaker.c",
        .device_class = DeviceClass::Input,
        .vendor_id = 0,
        .device_id = 0,
        .init_sequence = kSpeakerInit,
        .reset_sequence = kSpeakerInit,
    };
}

} // namespace pbsd::uda::speaker
