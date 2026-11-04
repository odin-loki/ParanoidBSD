export module pbsd.kde.frameworks.kconfig.kconfigwatcher;

import pbsd.core;

/// Wave 3 — KConfigWatcher group/key notification constants.
/// Upstream: kde/frameworks/kconfig/src/core/kconfigwatcher.cpp
export namespace pbsd::kde::frameworks::kconfig::kconfigwatcher {

inline constexpr const char kShortcutsGroup[] = "Shortcuts";
inline constexpr const char kGeneralGroup[] = "General";
inline constexpr unsigned kMaxKeys = 64;
inline constexpr unsigned kMaxKeyLen = 128;

struct ChangeSet {
    char group[128]{};
    char keys[kMaxKeys][kMaxKeyLen]{};
    unsigned key_count{0};
};

[[nodiscard]] inline bool is_shortcuts_group(const char* group) noexcept {
    if (group == nullptr) {
        return false;
    }
    return group[0] == 'S' && group[1] == 'h' && group[2] == 'o' && group[3] == 'r'
        && group[4] == 't' && group[5] == 'c' && group[6] == 'u' && group[7] == 't'
        && group[8] == 's' && group[9] == '\0';
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kconfig/src/core/kconfigwatcher.cpp";
}

} // namespace pbsd::kde::frameworks::kconfig::kconfigwatcher
