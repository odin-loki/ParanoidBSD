module;

#include <cstdio>
#include <cstring>

export module pbsd.kde.frameworks.kcoreaddons.kfileutils;

import pbsd.core;

/// Wave 3 — suggested filename logic (from KFileUtils::makeSuggestedName).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/io/kfileutils.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kfileutils {

inline constexpr unsigned kMaxNameLen = 512;

struct NameBuffer {
    char data[kMaxNameLen]{};
};

namespace detail {

[[nodiscard]] inline const char* find_last_dot(const char* name) noexcept {
    if (name == nullptr) {
        return nullptr;
    }
    const char* last = nullptr;
    for (const char* p = name; *p != '\0'; ++p) {
        if (*p == '.') {
            last = p;
        }
    }
    return last;
}

[[nodiscard]] inline bool has_number_suffix(const char* basename, unsigned& num) noexcept {
    if (basename == nullptr) {
        return false;
    }
    const char* open = std::strrchr(basename, '(');
    if (open == nullptr) {
        return false;
    }
    const char* close = std::strchr(open, ')');
    if (close == nullptr || close[1] != '\0') {
        return false;
    }
    num = 0;
    for (const char* p = open + 1; p < close; ++p) {
        if (*p < '0' || *p > '9') {
            return false;
        }
        num = num * 10 + static_cast<unsigned>(*p - '0');
    }
    return true;
}

} // namespace detail

[[nodiscard]] inline Status make_suggested_name(const char* old_name, NameBuffer& out) noexcept {
    if (old_name == nullptr) {
        return Status::Invalid;
    }
    const char* dot = detail::find_last_dot(old_name);
    char basename[kMaxNameLen]{};
    char suffix[kMaxNameLen]{};
    if (dot == old_name) {
        basename[0] = '.';
        basename[1] = '\0';
        std::strncpy(suffix, old_name, kMaxNameLen - 1);
    } else if (dot == nullptr) {
        std::strncpy(basename, old_name, kMaxNameLen - 1);
        suffix[0] = '\0';
    } else {
        const unsigned base_len = static_cast<unsigned>(dot - old_name);
        if (base_len >= kMaxNameLen) {
            return Status::NoMemory;
        }
        std::strncpy(basename, old_name, base_len);
        basename[base_len] = '\0';
        std::strncpy(suffix, dot, kMaxNameLen - 1);
    }
    unsigned num = 0;
    if (detail::has_number_suffix(basename, num)) {
        const int n = ::snprintf(out.data, kMaxNameLen, "%.*s (%u)%s",
                                    static_cast<int>(std::strlen(basename) - 4),
                                    basename, num + 1, suffix);
        return n > 0 && static_cast<unsigned>(n) < kMaxNameLen ? Status::Ok : Status::NoMemory;
    }
    const int n = ::snprintf(out.data, kMaxNameLen, "%s (1)%s", basename, suffix);
    return n > 0 && static_cast<unsigned>(n) < kMaxNameLen ? Status::Ok : Status::NoMemory;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/io/kfileutils.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kfileutils
