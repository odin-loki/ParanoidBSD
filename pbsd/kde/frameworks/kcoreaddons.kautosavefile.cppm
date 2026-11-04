module;

#include <cstring>

export module pbsd.kde.frameworks.kcoreaddons.kautosavefile;

import pbsd.core;

/// Wave 3 — autosave stale-file discovery (from kautosavefile.cpp).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/io/kautosavefile.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kautosavefile {

inline constexpr unsigned kNamePadding = 8;
inline constexpr unsigned kMaxNameLen = 255;
inline constexpr const char kStalefilesSuffix[] = "/stalefiles/";

struct StaleScan {
    char app_name[64]{};
    unsigned found{0};
};

[[nodiscard]] inline Status build_stale_path(const char* app, char* out, unsigned out_len) noexcept {
    if (app == nullptr || out == nullptr || out_len < 32) {
        return Status::Invalid;
    }
    std::strncpy(out, kStalefilesSuffix, out_len - 1);
    std::strncat(out, app, out_len - std::strlen(out) - 1);
    return Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/io/kautosavefile.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kautosavefile
