module;
#include <cstddef>

export module pbsd.stand.manifest;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/manifest.c
export namespace pbsd::stand::manifest {

inline constexpr const char kManifestPath[] = "/boot/manifest";

struct Entry {
    const char* name{};
    const char* hash{};
    std::size_t size{};
};

[[nodiscard]] inline Status validate_entry(const Entry& e) noexcept {
    if (e.name == nullptr || e.hash == nullptr || e.size == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::stand::manifest
