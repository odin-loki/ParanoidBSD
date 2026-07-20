export module pbsd.kde.kwin.textinput;

import pbsd.core;

/// Wave 3 pass 3 — Wayland text-input protocol stub.
/// Upstream: kde/kwin/src/wayland/textinput.cpp
export namespace pbsd::kde::kwin::textinput {

    inline constexpr const char kProtocolName[] = "zwp_text_input_v3";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/wayland/textinput.cpp";
}

} // namespace pbsd::kde::kwin::textinput
