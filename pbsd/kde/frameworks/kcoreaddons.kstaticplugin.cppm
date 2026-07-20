module;

#include <cstring>

export module pbsd.kde.frameworks.kcoreaddons.kstaticplugin;

import pbsd.core;

/// Wave 3 — static plugin registry stub (from KStaticPluginHelpers).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/plugin/kstaticpluginhelpers.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kstaticplugin {

inline constexpr unsigned kMaxDirectories = 16;
inline constexpr unsigned kMaxPluginsPerDir = 32;
inline constexpr unsigned kMaxIdLen = 128;
inline constexpr unsigned kMaxDirLen = 256;

struct PluginEntry {
    char id[kMaxIdLen]{};
    char directory[kMaxDirLen]{};
    bool present{false};
};

struct Registry {
    PluginEntry entries[kMaxDirectories * kMaxPluginsPerDir]{};
    unsigned count{0};
};

[[nodiscard]] inline Status register_plugin(Registry& reg, const char* directory,
                                            const char* plugin_id) noexcept {
    if (directory == nullptr || plugin_id == nullptr) {
        return Status::Invalid;
    }
    if (reg.count >= kMaxDirectories * kMaxPluginsPerDir) {
        return Status::NoMemory;
    }
    PluginEntry& e = reg.entries[reg.count];
    std::strncpy(e.directory, directory, kMaxDirLen - 1);
    std::strncpy(e.id, plugin_id, kMaxIdLen - 1);
    e.present = true;
    ++reg.count;
    return Status::Ok;
}

[[nodiscard]] inline bool find_by_id(const Registry& reg, const char* directory,
                                     const char* plugin_id) noexcept {
    if (directory == nullptr || plugin_id == nullptr) {
        return false;
    }
    for (unsigned i = 0; i < reg.count; ++i) {
        if (reg.entries[i].present
            && std::strcmp(reg.entries[i].directory, directory) == 0
            && std::strcmp(reg.entries[i].id, plugin_id) == 0) {
            return true;
        }
    }
    return false;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/plugin/kstaticpluginhelpers.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kstaticplugin
