module;

export module pbsd.compositor.wayland.abstract_drop_handler;

import pbsd.core;

/// Burst 13 — wl_data_device drop mime hints (native compositor).
export namespace pbsd::compositor::wayland::abstract_drop_handler {

inline constexpr const char kMimeTextPlain[] = "text/plain";
inline constexpr const char kMimeUriList[] = "text/uri-list";
inline constexpr const char kMimeXWayland[] = "application/x-wayland";

enum class DropAction : unsigned char {
    None = 0,
    Copy = 1,
    Move = 2,
};

[[nodiscard]] inline Status validate_action(unsigned action) noexcept {
    return action <= static_cast<unsigned>(DropAction::Move) ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::compositor::wayland::abstract_drop_handler
