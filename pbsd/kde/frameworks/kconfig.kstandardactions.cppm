export module pbsd.kde.frameworks.kconfig.kstandardactions;

import pbsd.core;

/// Wave 3 — standard action IDs (from kstandardactions.cpp).
/// Upstream: kde/frameworks/kconfig/src/gui/kstandardactions.cpp
export namespace pbsd::kde::frameworks::kconfig::kstandardactions {

enum class StandardAction : unsigned short {
    None = 0,
    Back,
    Forward,
    Home,
    Prior,
    Next,
    Preferences,
    AboutApp,
    HelpContents,
};

inline constexpr unsigned kActionCount = 9;

[[nodiscard]] inline bool is_navigation(StandardAction a) noexcept {
    return a == StandardAction::Back || a == StandardAction::Forward
        || a == StandardAction::Home;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kconfig/src/gui/kstandardactions.cpp";
}

} // namespace pbsd::kde::frameworks::kconfig::kstandardactions
