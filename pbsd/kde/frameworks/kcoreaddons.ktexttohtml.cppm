export module pbsd.kde.kcoreaddons.ktexttohtml;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (ktexttohtmltest.cpp).
/// Upstream: kde/frameworks/kcoreaddons/tests/ktexttohtmltest.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::ktexttohtml {

inline constexpr const char kBoldTag[] = "<b>";
inline constexpr const char kItalicTag[] = "<i>";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/tests/ktexttohtmltest.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::ktexttohtml
