module;

#include <cstdio>
#include <cstring>

export module pbsd.kde.frameworks.kcoreaddons.kbackup;

import pbsd.core;

/// Wave 3 — backup filename templates (from KBackup).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/io/kbackup.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kbackup {

inline constexpr unsigned kMaxPathLen = 4096;
inline constexpr const char kDefaultExtension[] = "~";

struct PathBuffer {
    char data[kMaxPathLen]{};
};

[[nodiscard]] inline Status simple_backup_path(const char* filename,
                                               const char* backup_dir,
                                               const char* extension,
                                               PathBuffer& out) noexcept {
    if (filename == nullptr) {
        return Status::Invalid;
    }
    const char* ext = extension != nullptr ? extension : kDefaultExtension;
    if (backup_dir == nullptr || backup_dir[0] == '\0') {
        const int n = snprintf(out.data, kMaxPathLen, "%s%s", filename, ext);
        return n > 0 && static_cast<unsigned>(n) < kMaxPathLen ? Status::Ok : Status::NoMemory;
    }
    const char* base = std::strrchr(filename, '/');
    base = base != nullptr ? base + 1 : filename;
    const int n = snprintf(out.data, kMaxPathLen, "%s/%s%s", backup_dir, base, ext);
    return n > 0 && static_cast<unsigned>(n) < kMaxPathLen ? Status::Ok : Status::NoMemory;
}

[[nodiscard]] inline Status numbered_backup_path(const char* filename,
                                                 const char* backup_dir,
                                                 const char* extension,
                                                 unsigned number,
                                                 PathBuffer& out) noexcept {
    if (filename == nullptr) {
        return Status::Invalid;
    }
    const char* ext = extension != nullptr ? extension : kDefaultExtension;
    const char* base = std::strrchr(filename, '/');
    base = base != nullptr ? base + 1 : filename;
    if (backup_dir == nullptr || backup_dir[0] == '\0') {
        const int n = snprintf(out.data, kMaxPathLen, "%s.%u%s", filename, number, ext);
        return n > 0 && static_cast<unsigned>(n) < kMaxPathLen ? Status::Ok : Status::NoMemory;
    }
    const int n = snprintf(out.data, kMaxPathLen, "%s/%s.%u%s", backup_dir, base, number, ext);
    return n > 0 && static_cast<unsigned>(n) < kMaxPathLen ? Status::Ok : Status::NoMemory;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/io/kbackup.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kbackup
