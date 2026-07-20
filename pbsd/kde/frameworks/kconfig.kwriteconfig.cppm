module;

#include <cstring>

export module pbsd.kde.frameworks.kconfig.kwriteconfig;

import pbsd.core;

/// Wave 3 — kwriteconfig CLI argument parsing (from kwriteconfig.cpp).
/// Upstream: kde/frameworks/kconfig/src/kreadconfig/kwriteconfig.cpp
export namespace pbsd::kde::frameworks::kconfig::kwriteconfig {

inline constexpr unsigned kMaxArgLen = 512;
inline constexpr const char kFileOption[] = "--file";
inline constexpr const char kGroupOption[] = "--group";
inline constexpr const char kKeyOption[] = "--key";
inline constexpr const char kTypeOption[] = "--type";

struct Request {
    char file[kMaxArgLen]{};
    char group[kMaxArgLen]{};
    char key[kMaxArgLen]{};
    char value[kMaxArgLen]{};
    char type[kMaxArgLen]{};
};

[[nodiscard]] inline Status set_value(Request& req, const char* value) noexcept {
    if (value == nullptr) {
        return Status::Invalid;
    }
    std::strncpy(req.value, value, kMaxArgLen - 1);
    req.value[kMaxArgLen - 1] = '\0';
    return Status::Ok;
}

[[nodiscard]] inline bool request_complete(const Request& req) noexcept {
    return req.file[0] != '\0' && req.group[0] != '\0' && req.key[0] != '\0'
        && req.value[0] != '\0';
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kconfig/src/kreadconfig/kwriteconfig.cpp";
}

} // namespace pbsd::kde::frameworks::kconfig::kwriteconfig
