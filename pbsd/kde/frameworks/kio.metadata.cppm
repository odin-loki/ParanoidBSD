module;

#include <cstring>

export module pbsd.kde.frameworks.kio.metadata;

import pbsd.core;

/// Wave 3 — KIO MetaData key constants (header-only upstream TU).
/// Upstream: kde/frameworks/kio/src/core/metadata.cpp
export namespace pbsd::kde::frameworks::kio::metadata {

inline constexpr const char kContentType[] = "content-type";
inline constexpr const char kContentLength[] = "content-length";
inline constexpr const char kLastModified[] = "last-modified";
inline constexpr const char kEntityId[] = "entity-id";
inline constexpr const char kPermissions[] = "permissions";
inline constexpr const char kMimeType[] = "mimeType";
inline constexpr const char kResponseCode[] = "responsecode";
inline constexpr const char kRedirectUrl[] = "redirecturl";

inline constexpr unsigned kMaxKeyLen = 128;
inline constexpr unsigned kMaxValueLen = 512;
inline constexpr unsigned kMaxEntries = 64;

struct Entry {
    char key[kMaxKeyLen]{};
    char value[kMaxValueLen]{};
};

struct Map {
    Entry entries[kMaxEntries]{};
    unsigned count{0};
};

[[nodiscard]] inline Status clear(Map& map) noexcept {
    map.count = 0;
    return Status::Ok;
}

[[nodiscard]] inline Status set(Map& map, const char* key, const char* value) noexcept {
    if (key == nullptr || value == nullptr) {
        return Status::Invalid;
    }
    if (map.count >= kMaxEntries) {
        return Status::NoMemory;
    }
    Entry& e = map.entries[map.count];
    std::memset(e.key, 0, kMaxKeyLen);
    std::memset(e.value, 0, kMaxValueLen);
    std::strncpy(e.key, key, kMaxKeyLen - 1);
    std::strncpy(e.value, value, kMaxValueLen - 1);
    ++map.count;
    return Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kio/src/core/metadata.cpp";
}

} // namespace pbsd::kde::frameworks::kio::metadata
