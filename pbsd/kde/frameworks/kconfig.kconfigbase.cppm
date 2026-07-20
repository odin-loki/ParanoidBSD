export module pbsd.kde.frameworks.kconfig.kconfigbase;

import pbsd.core;

/// Wave 3 — KConfigBase group path constants (from kconfigbase.cpp façade).
/// Upstream: kde/frameworks/kconfig/src/core/kconfigbase.cpp
export namespace pbsd::kde::frameworks::kconfig::kconfigbase {

inline constexpr unsigned kMaxGroupLen = 256;
inline constexpr unsigned kMaxKeyLen = 256;
inline constexpr const char kDefaultGroup[] = "";
inline constexpr const char kDesktopGroup[] = "Desktop Entry";
inline constexpr const char kGeneralGroup[] = "General";

enum class WriteConfigFlag : unsigned {
    None = 0,
    Persistent = 1,
    Sync = 2,
};

[[nodiscard]] inline bool is_valid_group_name(const char* group) noexcept {
    return group != nullptr;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kconfig/src/core/kconfigbase.cpp";
}

} // namespace pbsd::kde::frameworks::kconfig::kconfigbase
