export module pbsd.kde.tabbox_data;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (kwintabboxdata.cpp).
/// Upstream: kde/kwin/src/kcms/tabbox/kwintabboxdata.cpp
export namespace pbsd::kde::tabbox_data {

inline constexpr const char kConfigGroup[] = "TabBox";
inline constexpr unsigned kMaxLayouts = 8;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/kcms/tabbox/kwintabboxdata.cpp";
}

} // namespace pbsd::kde::tabbox_data
