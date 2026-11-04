export module pbsd.kde.plasma.volume.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.volume;

/// Burst 12 — Volume slider applet ↔ Aero widget chrome glue.
/// Upstream: kde/plasma-workspace/applets/volume/volume.cpp
export namespace pbsd::kde::plasma::volume::glue {

struct VolumeStyle {
    int default_volume{volume::kDefaultVolume};
    int slider_width{volume::kSliderWidth};
    int icon_size{22};
    float popup_opacity{0.82f};
    const char* volume_svg{aero::kVolumeSvg};
};

[[nodiscard]] inline VolumeStyle default_style() noexcept {
    return VolumeStyle{};
}

[[nodiscard]] inline int clamp_volume(int v) noexcept {
    if (v < 0) {
        return 0;
    }
    if (v > volume::kMaxVolume) {
        return volume::kMaxVolume;
    }
    return v;
}

[[nodiscard]] inline float opacity_for_volume(int v) noexcept {
    const int clamped = clamp_volume(v);
    return 0.72f + static_cast<float>(clamped) / static_cast<float>(volume::kMaxVolume) * 0.20f;
}

[[nodiscard]] inline Status validate_slider_width(int w) noexcept {
    if (w < 48 || w > 320) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return volume::upstream_path();
}

} // namespace pbsd::kde::plasma::volume::glue
