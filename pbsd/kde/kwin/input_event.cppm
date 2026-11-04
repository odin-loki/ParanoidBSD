export module pbsd.kde.kwin.input_event;

import pbsd.core;

/// Wave 3 — input event type tags.
/// Upstream: kde/kwin/src/input_event.cpp
export namespace pbsd::kde::kwin::input_event {

enum class Type : unsigned char { Keyboard, Pointer, Touch, Tablet, Switch, Unknown };

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/input_event.cpp";
}

} // namespace pbsd::kde::kwin::input_event
