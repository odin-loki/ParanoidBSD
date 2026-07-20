export module pbsd.theme.plasma.aero.icons;

import pbsd.core;

/// Wave 9 burst — Aero icon sizing constants.
export namespace pbsd::theme::plasma::aero::icons {

inline constexpr int kSmall = 16;
inline constexpr int kMedium = 22;
inline constexpr int kLarge = 32;
inline constexpr int kHuge = 48;

enum class Context : unsigned char {
    Panel,
    StartMenu,
    Dialog,
    Notification,
};

[[nodiscard]] inline int size_for(Context ctx) noexcept {
    switch (ctx) {
    case Context::Panel: return kMedium;
    case Context::StartMenu: return kLarge;
    case Context::Dialog: return kMedium;
    case Context::Notification: return kSmall;
    default: return kMedium;
    }
}

} // namespace pbsd::theme::plasma::aero::icons
