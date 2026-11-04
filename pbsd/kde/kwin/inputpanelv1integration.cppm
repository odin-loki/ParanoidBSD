export module pbsd.kde.kwin.inputpanelv1integration;

import pbsd.core;

/// Wave 3 pass 4 — Input panel v1 protocol stub.
/// Upstream: kde/kwin/src/inputpanelv1integration.cpp
export namespace pbsd::kde::kwin::inputpanelv1integration {

    inline constexpr const char kProtocolName[] = "zwp_input_panel_v1";
    inline constexpr unsigned kVersion = 1;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/inputpanelv1integration.cpp";
}

} // namespace pbsd::kde::kwin::inputpanelv1integration
