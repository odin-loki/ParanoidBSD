export module pbsd.kde.kpackage_decoration;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (decoration.cpp).
/// Upstream: kde/kwin/src/plugins/kpackage/decoration/decoration.cpp
export namespace pbsd::kde::kpackage_decoration {

inline constexpr const char kPackageRoot[] = "aurorae/themes";
inline constexpr const char kConfigFile[] = "config.json";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/kpackage/decoration/decoration.cpp";
}

} // namespace pbsd::kde::kpackage_decoration
