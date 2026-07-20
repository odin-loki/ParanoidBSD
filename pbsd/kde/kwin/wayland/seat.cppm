export module pbsd.kde.kwin.wayland.seat;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Burst 17 — wl_seat protocol glue.
/// Upstream: kde/kwin/src/wayland/seat.cpp
export namespace pbsd::kde::kwin::wayland::seat {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_SEAT",
    "kwin_seat",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/wayland/seat.cpp",
};

inline constexpr const char kInterface[] = "wl_seat";
inline constexpr int kVersion = 8;

enum class Capability : unsigned char {
    Pointer = 1,
    Keyboard = 2,
    Touch = 4,
};

[[nodiscard]] inline Status validate_capabilities(unsigned caps) noexcept {
    if (caps == 0 || caps > 7) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/wayland/seat.cpp";
}

} // namespace pbsd::kde::kwin::wayland::seat
