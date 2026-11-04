export module pbsd.kde.kio.kfilewidgetdocktitlebar;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (kfilewidgetdocktitlebar.cpp).
/// Upstream: kde/frameworks/kio/src/filewidgets/kfilewidgetdocktitlebar.cpp
export namespace pbsd::kde::frameworks::kio::kfilewidgetdocktitlebar {

inline constexpr const char kDockTitle[] = "Places";
inline constexpr unsigned kTitleHeight = 24;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kio/src/filewidgets/kfilewidgetdocktitlebar.cpp";
}

} // namespace pbsd::kde::frameworks::kio::kfilewidgetdocktitlebar
