module;

#include <cstring>

export module pbsd.kde.frameworks.kcoreaddons.knetworkmounts;

import pbsd.core;
import pbsd.kde.frameworks.kcoreaddons.filesystemtype;

/// Wave 3 — network mount slow-path keys (from KNetworkMounts).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/io/knetworkmounts.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::knetworkmounts {

enum class MountType : unsigned {
    NfsPaths = 0,
    SmbPaths,
    SymlinkDirectory,
    SymlinkToNetworkMount,
};

enum class MountOption : unsigned {
    EnableOptimizations = 0,
    SkipThumbnail,
    SkipMetadata,
};

inline constexpr const char kConfigFileSuffix[] = "network_mounts";
inline constexpr const char kEnableOptimizationsKey[] = "EnableOptimizations";
inline constexpr unsigned kMaxPathLen = 4096;
inline constexpr unsigned kMaxPaths = 64;

struct PathEntry {
    char path[kMaxPathLen]{};
    MountType type{MountType::NfsPaths};
};

struct PathList {
    PathEntry entries[kMaxPaths]{};
    unsigned count{0};
    bool optimizations_enabled{false};
};

[[nodiscard]] inline const char* enum_to_string(MountType type) noexcept {
    switch (type) {
    case MountType::NfsPaths:
        return "NfsPaths";
    case MountType::SmbPaths:
        return "SmbPaths";
    case MountType::SymlinkDirectory:
        return "SymlinkDirectory";
    case MountType::SymlinkToNetworkMount:
        return "SymlinkToNetworkMount";
    }
    return "NfsPaths";
}

[[nodiscard]] inline Status ensure_trailing_slash(char* path, unsigned cap) noexcept {
    if (path == nullptr || cap == 0) {
        return Status::Invalid;
    }
    const unsigned len = static_cast<unsigned>(std::strlen(path));
    if (len == 0 || path[len - 1] == '/') {
        return Status::Ok;
    }
    if (len + 1 >= cap) {
        return Status::NoMemory;
    }
    path[len] = '/';
    path[len + 1] = '\0';
    return Status::Ok;
}

[[nodiscard]] inline bool is_slow_path(const PathList& list, const char* path) noexcept {
    if (path == nullptr || !list.optimizations_enabled) {
        return false;
    }
    for (unsigned i = 0; i < list.count; ++i) {
        const char* entry = list.entries[i].path;
        unsigned j = 0;
        while (path[j] != '\0' && entry[j] != '\0' && path[j] == entry[j]) {
            ++j;
        }
        if (j > 0 && entry[j] == '\0') {
            return true;
        }
    }
    return filesystemtype::is_network_type(filesystemtype::type_from_name(path));
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/io/knetworkmounts.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::knetworkmounts
