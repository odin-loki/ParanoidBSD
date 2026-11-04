export module pbsd.kde.kwin.wayland.textinput_v3;

import pbsd.core;

/// Wave 3 pass 4 — Text input v3 protocol name.
/// Upstream: kde/kwin/src/wayland/textinput.cpp
export namespace pbsd::kde::kwin::wayland::textinput_v3 {

    inline constexpr const char kProtocolName[] = "zwp_text_input_v3";
    inline constexpr unsigned kVersion = 3;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/wayland/textinput.cpp";
}

} // namespace pbsd::kde::kwin::wayland::textinput_v3
