export module pbsd.theme.plasma.aero.cursors;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 12 — Aero cursor theme size and hotspot tokens.
export namespace pbsd::theme::plasma::aero::cursors {

enum class Kind : unsigned char {
    Default,
    Pointer,
    Text,
    Resize,
    Wait,
    Crosshair,
};

struct CursorSpec {
    int size{24};
    int hotspot_x{0};
    int hotspot_y{0};
    const char* theme_name{"Breeze"};
};

inline constexpr CursorSpec kDefault{24, 0, 0, "Breeze"};
inline constexpr CursorSpec kPointer{24, 4, 2, "Breeze"};
inline constexpr CursorSpec kText{24, 8, 8, "Breeze"};
inline constexpr CursorSpec kWait{32, 16, 16, "Breeze"};

[[nodiscard]] inline CursorSpec for_kind(Kind k) noexcept {
    switch (k) {
    case Kind::Pointer:
        return kPointer;
    case Kind::Text:
        return kText;
    case Kind::Wait:
        return kWait;
    default:
        return kDefault;
    }
}

[[nodiscard]] inline int scaled_size(int base, float scale) noexcept {
    if (scale <= 0.0f) {
        return base;
    }
    return static_cast<int>(static_cast<float>(base) * scale + 0.5f);
}

[[nodiscard]] inline Status validate_size(int px) noexcept {
    if (px < 8 || px > 64) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::theme::plasma::aero::cursors
