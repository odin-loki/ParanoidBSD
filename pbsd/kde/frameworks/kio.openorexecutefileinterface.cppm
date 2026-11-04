export module pbsd.kde.kio.openorexecutefileinterface;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (openorexecutefileinterface.cpp).
/// Upstream: kde/frameworks/kio/src/gui/openorexecutefileinterface.cpp
export namespace pbsd::kde::frameworks::kio::openorexecutefileinterface {

inline constexpr const char kInterfaceIid[] = "org.kde.kio.openOrExecuteFile/1.0";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kio/src/gui/openorexecutefileinterface.cpp";
}

} // namespace pbsd::kde::frameworks::kio::openorexecutefileinterface
