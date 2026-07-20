module;
#include <cstdint>

export module pbsd.pkg.manifest;

import pbsd.core;

/// Wave 9 — package manifest schema (content-addressed).
export namespace pbsd::pkg::manifest {

enum class EntryType : unsigned char {
    File      = 0,
    Directory = 1,
    Symlink   = 2,
    Device    = 3,
};

struct Entry {
    EntryType type{EntryType::File};
    char path[256]{};
    char digest_hex[65]{};
    unsigned long long size{};
};

[[nodiscard]] inline Status validate_entry(Entry const& e) noexcept {
    if (e.path[0] == '\0') {
        return Status::Invalid;
    }
    if (e.type == EntryType::File && e.size == 0 && e.digest_hex[0] == '\0') {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::pkg::manifest
