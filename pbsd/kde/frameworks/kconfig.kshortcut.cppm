export module pbsd.kde.frameworks.kconfig.kshortcut;

import pbsd.core;

/// Wave 3 — hand port constants (kshortcut.cpp).
/// Upstream: kde/frameworks/kconfig/src/gui/kshortcut.cpp
export namespace pbsd::kde::frameworks::kconfig::kshortcut {

inline constexpr const char kPrimarySequenceKey[] = "Primary";
inline constexpr const char kDefaultGroup[] = "Shortcuts";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kconfig/src/gui/kshortcut.cpp";
}

} // namespace pbsd::kde::frameworks::kconfig::kshortcut
