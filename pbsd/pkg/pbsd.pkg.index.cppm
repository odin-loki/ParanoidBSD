module;
#include <cstdint>

export module pbsd.pkg.index;

import pbsd.core;
import pbsd.pkg.manifest;

/// Burst 12 — package index lookup (name → digest).
export namespace pbsd::pkg::index {

inline constexpr unsigned kMaxEntries = 256;

struct IndexEntry {
    char name[64]{};
    char version[32]{};
    char digest_hex[65]{};
    manifest::EntryType kind{manifest::EntryType::File};
};

struct PackageIndex {
    unsigned count{0};
    IndexEntry entries[kMaxEntries]{};
};

[[nodiscard]] inline Status lookup(PackageIndex const& idx, char const* name,
                                   IndexEntry& out) noexcept {
    if (name == nullptr || name[0] == '\0') {
        return Status::Invalid;
    }
    for (unsigned i = 0; i < idx.count; ++i) {
        bool match = true;
        for (unsigned j = 0; name[j] != '\0' || idx.entries[i].name[j] != '\0'; ++j) {
            if (name[j] != idx.entries[i].name[j]) {
                match = false;
                break;
            }
        }
        if (match) {
            out = idx.entries[i];
            return Status::Ok;
        }
    }
    return Status::NotFound;
}

[[nodiscard]] inline Status insert(PackageIndex& idx, IndexEntry const& e) noexcept {
    if (idx.count >= kMaxEntries || e.name[0] == '\0' || e.digest_hex[0] == '\0') {
        return Status::Invalid;
    }
    idx.entries[idx.count++] = e;
    return Status::Ok;
}

} // namespace pbsd::pkg::index
