export module pbsd.kde.frameworks.kconfig.kconfigskeleton;

import pbsd.core;

/// Wave 3 — KConfigSkeleton item type tags (from kconfigskeleton.cpp).
/// Upstream: kde/frameworks/kconfig/src/gui/kconfigskeleton.cpp
export namespace pbsd::kde::frameworks::kconfig::kconfigskeleton {

enum class ItemKind : unsigned char { Color, Font, Generic, Unknown };

inline constexpr unsigned kMaxItemNameLen = 128;
inline constexpr unsigned kMaxGroupLen = 128;
inline constexpr unsigned kMaxKeyLen = 128;

struct ItemRef {
    ItemKind kind{ItemKind::Unknown};
    char group[kMaxGroupLen]{};
    char key[kMaxKeyLen]{};
    char name[kMaxItemNameLen]{};
};

[[nodiscard]] inline ItemKind kind_for_name(const char* name) noexcept {
    if (name == nullptr) {
        return ItemKind::Unknown;
    }
    if (name[0] == 'C' && name[1] == 'o' && name[2] == 'l' && name[3] == 'o' && name[4] == 'r') {
        return ItemKind::Color;
    }
    if (name[0] == 'F' && name[1] == 'o' && name[2] == 'n' && name[3] == 't') {
        return ItemKind::Font;
    }
    return ItemKind::Generic;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kconfig/src/gui/kconfigskeleton.cpp";
}

} // namespace pbsd::kde::frameworks::kconfig::kconfigskeleton
