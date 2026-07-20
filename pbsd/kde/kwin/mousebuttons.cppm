export module pbsd.kde.kwin.mousebuttons;

import pbsd.core;

/// Wave 3 pass 4 — Linux input button codes (from mousebuttons.cpp).
/// Upstream: kde/kwin/src/mousebuttons.cpp
export namespace pbsd::kde::kwin::mousebuttons {

inline constexpr unsigned kBtnLeft = 0x110;
inline constexpr unsigned kBtnMiddle = 0x111;
inline constexpr unsigned kBtnRight = 0x112;
inline constexpr unsigned kBtnSide = 0x110 + 4;
inline constexpr unsigned kBtnExtra = 0x110 + 5;
inline constexpr unsigned kBtnForward = 0x110 + 6;
inline constexpr unsigned kBtnBack = 0x110 + 7;
inline constexpr unsigned kExtraButtonBase = 0x118;

[[nodiscard]] inline bool is_primary(unsigned btn) noexcept { return btn == kBtnLeft; }

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/mousebuttons.cpp";
}

} // namespace pbsd::kde::kwin::mousebuttons
