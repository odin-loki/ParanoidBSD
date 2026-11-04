export module pbsd.kde.kxmlgui.kbugreport;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (kbugreporttest.cpp).
/// Upstream: kde/frameworks/kxmlgui/tests/kbugreporttest.cpp
export namespace pbsd::kde::frameworks::kxmlgui::kbugreport {

inline constexpr const char kBugReportUrl[] = "https://bugs.kde.org";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kxmlgui/tests/kbugreporttest.cpp";
}

} // namespace pbsd::kde::frameworks::kxmlgui::kbugreport
