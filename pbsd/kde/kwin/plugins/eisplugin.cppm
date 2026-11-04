export module pbsd.kde.eisplugin;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (eisplugin.cpp).
/// Upstream: kde/kwin/src/plugins/eis/eisplugin.cpp
export namespace pbsd::kde::eisplugin {

inline constexpr const char kEisPluginId[] = "eis";
inline constexpr const char kInterfaceVersion[] = "1.0";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/eis/eisplugin.cpp";
}

} // namespace pbsd::kde::eisplugin
