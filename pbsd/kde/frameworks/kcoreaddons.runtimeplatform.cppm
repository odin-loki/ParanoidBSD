module;

#include <cstdlib>
#include <cstring>

export module pbsd.kde.frameworks.kcoreaddons.runtimeplatform;

import pbsd.core;

/// Wave 3 — PLASMA_PLATFORM env parsing (from KRuntimePlatform, Qt-free).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/util/kruntimeplatform.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::runtimeplatform {

inline constexpr unsigned kMaxPlatformLen = 256;
inline constexpr unsigned kMaxSegments = 16;
inline constexpr const char kEnvVar[] = "PLASMA_PLATFORM";

struct PlatformList {
    char segments[kMaxSegments][kMaxPlatformLen]{};
    unsigned count{0};
};

namespace detail {

[[nodiscard]] inline unsigned segment_len(const char* start, const char* end) noexcept {
    if (start == nullptr || end == nullptr || end < start) {
        return 0;
    }
    return static_cast<unsigned>(end - start);
}

[[nodiscard]] inline Status parse(const char* env, PlatformList& out) noexcept {
    out.count = 0;
    if (env == nullptr || env[0] == '\0') {
        return Status::Ok;
    }

    const char* cursor = env;
    while (*cursor != '\0' && out.count < kMaxSegments) {
        while (*cursor == ':') {
            ++cursor;
        }
        if (*cursor == '\0') {
            break;
        }
        const char* start = cursor;
        while (*cursor != '\0' && *cursor != ':') {
            ++cursor;
        }
        const unsigned len = segment_len(start, cursor);
        if (len == 0) {
            continue;
        }
        const unsigned copy = len < kMaxPlatformLen - 1 ? len : kMaxPlatformLen - 1;
        std::memset(out.segments[out.count], 0, kMaxPlatformLen);
        std::strncpy(out.segments[out.count], start, copy);
        ++out.count;
    }
    return Status::Ok;
}

} // namespace detail

[[nodiscard]] inline Status runtime_platform(PlatformList& out) noexcept {
    return detail::parse(std::getenv(kEnvVar), out);
}

[[nodiscard]] inline bool has_segment(const PlatformList& list, const char* segment) noexcept {
    if (segment == nullptr) {
        return false;
    }
    for (unsigned i = 0; i < list.count; ++i) {
        if (std::strcmp(list.segments[i], segment) == 0) {
            return true;
        }
    }
    return false;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/util/kruntimeplatform.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::runtimeplatform
