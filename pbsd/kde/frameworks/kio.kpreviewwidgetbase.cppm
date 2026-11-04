export module pbsd.kde.kio.kpreviewwidgetbase;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (kpreviewwidgetbase.cpp).
/// Upstream: kde/frameworks/kio/src/filewidgets/kpreviewwidgetbase.cpp
export namespace pbsd::kde::frameworks::kio::kpreviewwidgetbase {

inline constexpr unsigned kPreviewMaxSize = 512;
inline constexpr unsigned kPreviewMinSize = 64;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kio/src/filewidgets/kpreviewwidgetbase.cpp";
}

} // namespace pbsd::kde::frameworks::kio::kpreviewwidgetbase
