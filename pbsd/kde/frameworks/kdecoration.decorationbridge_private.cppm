export module pbsd.kde.kdecoration.decorationbridge_private;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (decorationbridge.cpp).
/// Upstream: kde/frameworks/kdecoration/src/private/decorationbridge.cpp
export namespace pbsd::kde::frameworks::kdecoration::decorationbridge_private {

inline constexpr const char kBridgeIid[] = "org.kde.kdecoration2/1.0";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kdecoration/src/private/decorationbridge.cpp";
}

} // namespace pbsd::kde::frameworks::kdecoration::decorationbridge_private
