module;

#include <cstring>

export module pbsd.kde.frameworks.kconfig.kreadconfig;

import pbsd.core;

/// Wave 3 — kreadconfig CLI argument parsing (from kreadconfig.cpp).
/// Upstream: kde/frameworks/kconfig/src/kreadconfig/kreadconfig.cpp
export namespace pbsd::kde::frameworks::kconfig::kreadconfig {

inline constexpr unsigned kMaxArgLen = 512;
inline constexpr const char kFileOption[] = "--file";
inline constexpr const char kGroupOption[] = "--group";
inline constexpr const char kKeyOption[] = "--key";
inline constexpr const char kDefaultOption[] = "--default";

struct Request {
    char file[kMaxArgLen]{};
    char group[kMaxArgLen]{};
    char key[kMaxArgLen]{};
    char default_value[kMaxArgLen]{};
    bool has_default{false};
};

[[nodiscard]] inline Status parse_option(const char* flag, const char* value,
                                           char* dest, unsigned cap) noexcept {
    if (flag == nullptr || value == nullptr || dest == nullptr || cap == 0) {
        return Status::Invalid;
    }
    std::strncpy(dest, value, cap - 1);
    dest[cap - 1] = '\0';
    return Status::Ok;
}

[[nodiscard]] inline bool request_complete(const Request& req) noexcept {
    return req.file[0] != '\0' && req.group[0] != '\0' && req.key[0] != '\0';
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kconfig/src/kreadconfig/kreadconfig.cpp";
}

} // namespace pbsd::kde::frameworks::kconfig::kreadconfig
