module;

#if defined(__FreeBSD__)
#include <dlfcn.h>
#endif

#include <cstdio>
#include <cstring>
export module pbsd.kde.frameworks.kcoreaddons.klibexec;

import pbsd.core;

/// Wave 3 — libexec path resolution (FreeBSD dladdr path from KLibexec).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/util/klibexec.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::klibexec {

inline constexpr unsigned kMaxPathLen = 4096;
inline constexpr unsigned kMaxCandidates = 8;

struct PathBuffer {
    char data[kMaxPathLen]{};
};

struct CandidateList {
    PathBuffer paths[kMaxCandidates]{};
    unsigned count{0};
};

namespace detail {

#if defined(__FreeBSD__)

[[nodiscard]] inline Status library_path_from_address(void* address, PathBuffer& out) noexcept {
    if (address == nullptr) {
        return Status::Invalid;
    }
    Dl_info info{};
    if (dladdr(address, &info) == 0 || info.dli_fname == nullptr) {
        return Status::NotFound;
    }
    std::strncpy(out.data, info.dli_fname, kMaxPathLen - 1);
    return Status::Ok;
}

#else

[[nodiscard]] inline Status library_path_from_address(void*, PathBuffer& out) noexcept {
    out.data[0] = '\0';
    return Status::NotImplemented;
}

#endif

} // namespace detail

[[nodiscard]] inline Status path_from_address(const char* relative, void* address,
                                              PathBuffer& out) noexcept {
    if (relative == nullptr) {
        return Status::Invalid;
    }
    PathBuffer library{};
    const Status st = detail::library_path_from_address(address, library);
    if (st != Status::Ok) {
        return st;
    }
    const int n = snprintf(out.data, kMaxPathLen, "%s/%s", library.data, relative);
    if (n <= 0 || static_cast<unsigned>(n) >= kMaxPathLen) {
        return Status::NoMemory;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status path_candidates(const char* relative, CandidateList& out) noexcept {
    if (relative == nullptr) {
        return Status::Invalid;
    }
    out.count = 0;
    const char* prefixes[] = {"./libexec", "/usr/local/libexec/kf6", "/usr/libexec/kf6"};
    for (const char* prefix : prefixes) {
        if (out.count >= kMaxCandidates) {
            break;
        }
        snprintf(out.paths[out.count].data, kMaxPathLen, "%s/%s", prefix, relative);
        ++out.count;
    }
    if (out.count < kMaxCandidates) {
        std::strncpy(out.paths[out.count].data, relative, kMaxPathLen - 1);
        ++out.count;
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/util/klibexec.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::klibexec
