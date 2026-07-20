module;

#include <cstring>

export module pbsd.kde.frameworks.kconfig.kdesktopfileaction;

import pbsd.core;

/// Wave 3 — desktop file action separator + field keys.
/// Upstream: kde/frameworks/kconfig/src/core/kdesktopfileaction.cpp
export namespace pbsd::kde::frameworks::kconfig::kdesktopfileaction {

inline constexpr const char kSeparatorKey[] = "_SEPARATOR_";
inline constexpr unsigned kMaxFieldLen = 512;

struct Action {
    char internal_key[kMaxFieldLen]{};
    char name[kMaxFieldLen]{};
    char icon[kMaxFieldLen]{};
    char exec[kMaxFieldLen]{};
    char desktop_path[kMaxFieldLen]{};
};

[[nodiscard]] inline bool is_separator(const char* key) noexcept {
    if (key == nullptr) {
        return false;
    }
    return std::strcmp(key, kSeparatorKey) == 0;
}

[[nodiscard]] inline Status init_action(Action& a, const char* key, const char* name,
                                        const char* icon, const char* exec,
                                        const char* path) noexcept {
    if (key == nullptr || name == nullptr) {
        return Status::Invalid;
    }
    std::strncpy(a.internal_key, key, kMaxFieldLen - 1);
    std::strncpy(a.name, name, kMaxFieldLen - 1);
    if (icon != nullptr) {
        std::strncpy(a.icon, icon, kMaxFieldLen - 1);
    }
    if (exec != nullptr) {
        std::strncpy(a.exec, exec, kMaxFieldLen - 1);
    }
    if (path != nullptr) {
        std::strncpy(a.desktop_path, path, kMaxFieldLen - 1);
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kconfig/src/core/kdesktopfileaction.cpp";
}

} // namespace pbsd::kde::frameworks::kconfig::kdesktopfileaction
