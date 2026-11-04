export module pbsd.kde.frameworks.kcoreaddons.kstaticpluginhelpers;

import pbsd.core;

/// Wave 3 — static plugin registry helpers (from kstaticpluginhelpers.cpp).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/plugin/kstaticpluginhelpers.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kstaticpluginhelpers {

inline constexpr unsigned kMaxPluginIdLen = 256;
inline constexpr unsigned kMaxPluginsPerDir = 64;

struct PluginSlot {
    char id[kMaxPluginIdLen]{};
    bool occupied{false};
};

struct DirectoryMap {
    PluginSlot slots[kMaxPluginsPerDir]{};
    unsigned count{0};
};

[[nodiscard]] inline Status register_plugin(DirectoryMap& map, const char* id) noexcept {
    if (id == nullptr || map.count >= kMaxPluginsPerDir) {
        return map.count >= kMaxPluginsPerDir ? Status::NoMemory : Status::Invalid;
    }
    auto& slot = map.slots[map.count++];
    for (unsigned i = 0; i < kMaxPluginIdLen - 1 && id[i]; ++i) {
        slot.id[i] = id[i];
    }
    slot.occupied = true;
    return Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/plugin/kstaticpluginhelpers.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kstaticpluginhelpers
