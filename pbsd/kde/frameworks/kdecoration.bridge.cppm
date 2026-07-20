export module pbsd.kde.frameworks.kdecoration.bridge;

import pbsd.core;

/// Wave 3 pass 3 — KDecoration private bridge stub.
/// Upstream: kde/frameworks/kdecoration/src/private/decorationbridge.cpp
export namespace pbsd::kde::frameworks::kdecoration::bridge {

    inline constexpr const char kBridgeId[] = "org.kde.kdecoration.bridge";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kdecoration/src/private/decorationbridge.cpp";
}

} // namespace pbsd::kde::frameworks::kdecoration::bridge
