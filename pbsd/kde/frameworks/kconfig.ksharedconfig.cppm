export module pbsd.kde.frameworks.kconfig.ksharedconfig;

import pbsd.core;

/// Wave 3 — shared config registry constants (from ksharedconfig.cpp).
/// Upstream: kde/frameworks/kconfig/src/core/ksharedconfig.cpp
export namespace pbsd::kde::frameworks::kconfig::ksharedconfig {

inline constexpr unsigned kMaxConfigs = 32;
inline constexpr const char kGlobalConfigName[] = "kdeglobals";
inline constexpr const char kPlasmaConfigName[] = "plasma-org.kde.plasma.desktop-appletsrc";
inline constexpr const char kKWinConfigName[] = "kwinrc";

struct ConfigSlot {
    const char* name{nullptr};
    bool test_mode{false};
};

struct Registry {
    ConfigSlot slots[kMaxConfigs]{};
    unsigned count{0};
    bool was_test_mode{false};
};

[[nodiscard]] inline Status register_config(Registry& reg, const char* name) noexcept {
    if (name == nullptr || reg.count >= kMaxConfigs) {
        return name == nullptr ? Status::Invalid : Status::NoMemory;
    }
    reg.slots[reg.count].name = name;
    reg.slots[reg.count].test_mode = reg.was_test_mode;
    ++reg.count;
    return Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kconfig/src/core/ksharedconfig.cpp";
}

} // namespace pbsd::kde::frameworks::kconfig::ksharedconfig
