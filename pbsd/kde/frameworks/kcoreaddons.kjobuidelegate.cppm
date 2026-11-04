export module pbsd.kde.frameworks.kcoreaddons.kjobuidelegate;

import pbsd.core;

/// Wave 3 — KJobUiDelegate capability flags.
/// Upstream: kde/frameworks/kcoreaddons/src/lib/jobs/kjobuidelegate.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kjobuidelegate {

enum class Capability : unsigned char { None, ShowErrorMessage, ShowInfoMessage, All };

[[nodiscard]] inline bool shows_errors(Capability c) noexcept {
    return c == Capability::ShowErrorMessage || c == Capability::All;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/jobs/kjobuidelegate.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kjobuidelegate
