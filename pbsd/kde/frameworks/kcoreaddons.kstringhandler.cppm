module;

#include <cstring>

export module pbsd.kde.frameworks.kcoreaddons.kstringhandler;

import pbsd.core;

/// Wave 3 — squeeze helpers (from KStringHandler::{l,c,r}squeeze).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/text/kstringhandler.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kstringhandler {

inline constexpr const char kEllipsis[] = "...";
inline constexpr unsigned kMaxOutLen = 512;

[[nodiscard]] inline Status lsqueeze(const char* str, int maxlen, char* out,
                                   unsigned out_len) noexcept {
    if (str == nullptr || out == nullptr || out_len == 0) {
        return Status::Invalid;
    }
    const int len = static_cast<int>(std::strlen(str));
    if (len <= maxlen) {
        std::strncpy(out, str, out_len - 1);
        return Status::Ok;
    }
    const int part = maxlen - 3;
    if (part <= 0) {
        return Status::Invalid;
    }
    std::strncpy(out, kEllipsis, out_len - 1);
    std::strncat(out, str + len - part, out_len - std::strlen(out) - 1);
    return Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/text/kstringhandler.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kstringhandler
