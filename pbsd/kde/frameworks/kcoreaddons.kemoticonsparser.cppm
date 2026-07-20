export module pbsd.kde.frameworks.kcoreaddons.kemoticonsparser;

import pbsd.core;

/// Wave 3 — emoticon token delimiters (from kemoticonsparser.cpp).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/text/kemoticonsparser.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kemoticonsparser {

inline constexpr char kOpenParen = '(';
inline constexpr char kCloseParen = ')';
inline constexpr char kColon = ':';

[[nodiscard]] inline bool is_delimiter(char c) noexcept {
    return c == kOpenParen || c == kCloseParen || c == kColon;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/text/kemoticonsparser.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kemoticonsparser
