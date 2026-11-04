module;

export module pbsd.compositor.wayland.layer.popup;

import pbsd.core;

/// Burst 16 — wlr-layer-shell-unstable-v1.xml (native compositor).
export namespace pbsd::compositor::wayland::layer::popup {

inline constexpr const char kPopupInterface[] = "zwlr_layer_surface_v1";

enum class PopupRequest : unsigned char {
    GetPopup = 5,
    Destroy = 7,
};

[[nodiscard]] inline bool is_popup_request(unsigned opcode) noexcept {
    return opcode == static_cast<unsigned>(PopupRequest::GetPopup)
        || opcode == static_cast<unsigned>(PopupRequest::Destroy);
}

} // namespace pbsd::compositor::wayland::layer::popup
