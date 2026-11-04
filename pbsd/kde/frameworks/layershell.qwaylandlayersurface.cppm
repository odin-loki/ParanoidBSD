export module pbsd.kde.layershell.qwaylandlayersurface;

import pbsd.core;
import pbsd.kde.layershell.layershellintegration;

/// Burst 13 — layer-shell-qt surface role stub.
/// Upstream: kde/frameworks/layer-shell-qt/src/qwaylandlayersurface.cpp
export namespace pbsd::kde::layershell::qwaylandlayersurface {

inline constexpr const char kInterface[] = "zwlr_layer_surface_v1";
inline constexpr int kVersion = 4;

enum class Layer : unsigned char {
    Background = 0,
    Bottom = 1,
    Top = 2,
    Overlay = 3,
};

enum class Request : unsigned char {
    Destroy = 0,
    SetSize = 1,
    SetAnchor = 2,
    SetExclusiveZone = 3,
    SetMargin = 4,
    SetKeyboardInteractivity = 5,
    GetPopup = 6,
    AckConfigure = 7,
};

[[nodiscard]] inline const char* shell_surface_name() noexcept {
    return ::pbsd::kde::frameworks::layershell::layershellintegration::kShellSurface;
}

[[nodiscard]] inline Status validate_layer(unsigned layer) noexcept {
    return layer <= static_cast<unsigned>(Layer::Overlay) ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline Status validate_exclusive_zone(int zone) noexcept {
    if (zone < -1 || zone > 512) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/layer-shell-qt/src/qwaylandlayersurface.cpp";
}

} // namespace pbsd::kde::layershell::qwaylandlayersurface
