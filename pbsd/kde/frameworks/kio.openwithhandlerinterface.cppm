export module pbsd.kde.kio.openwithhandlerinterface;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (openwithhandlerinterface.cpp).
/// Upstream: kde/frameworks/kio/src/gui/openwithhandlerinterface.cpp
export namespace pbsd::kde::frameworks::kio::openwithhandlerinterface {

inline constexpr const char kInterfaceIid[] = "org.kde.kio.openWithHandler/1.0";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kio/src/gui/openwithhandlerinterface.cpp";
}

} // namespace pbsd::kde::frameworks::kio::openwithhandlerinterface
