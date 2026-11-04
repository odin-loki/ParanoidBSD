module;
#include <cstdint>

export module pbsd.compositor.wayland.layer;

import pbsd.core;

/// PROVENANCE: wlr-layer-shell-unstable-v1.xml + kde/frameworks/layer-shell-qt
export namespace pbsd::compositor::wayland::layer {

enum class ShellRequest : unsigned int {
    GetLayerSurface = 0,
    Destroy         = 1,
};

enum class LayerSurfaceRequest : unsigned int {
    SetSize         = 0,
    SetAnchor       = 1,
    SetExclusiveZone = 2,
    SetMargin       = 3,
    SetKeyboardInteractivity = 4,
    GetPopup        = 5,
    AckConfigure    = 6,
    Destroy         = 7,
};

enum class LayerSurfaceEvent : unsigned int {
    Configure = 0,
    Closed    = 1,
};

enum class Layer : unsigned int {
    Background = 0,
    Bottom     = 1,
    Top        = 2,
    Overlay    = 3,
};

enum class Anchor : unsigned int {
    None   = 0,
    Top    = 1,
    Bottom = 2,
    Left   = 4,
    Right  = 8,
};

[[nodiscard]] constexpr Anchor operator|(Anchor a, Anchor b) noexcept {
    return static_cast<Anchor>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}

enum class KeyboardInteractivity : unsigned int {
    None     = 0,
    Exclusive = 1,
    OnDemand = 2,
};

struct ConfigureEvent {
    std::uint32_t width{};
    std::uint32_t height{};
    std::uint32_t serial{};
};

[[nodiscard]] inline bool is_layer_shell_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(ShellRequest::Destroy);
}

[[nodiscard]] inline bool is_layer_surface_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(LayerSurfaceRequest::Destroy);
}

[[nodiscard]] inline Status validate_anchor(unsigned anchor) noexcept {
    if (anchor > (static_cast<unsigned>(Anchor::Top) | static_cast<unsigned>(Anchor::Bottom)
                  | static_cast<unsigned>(Anchor::Left) | static_cast<unsigned>(Anchor::Right))) {
        return Status::Protocol;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_layer(unsigned layer) noexcept {
    if (layer > static_cast<unsigned>(Layer::Overlay)) {
        return Status::Protocol;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_configure(ConfigureEvent const& c) noexcept {
    if (c.width == 0 && c.height == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::compositor::wayland::layer
