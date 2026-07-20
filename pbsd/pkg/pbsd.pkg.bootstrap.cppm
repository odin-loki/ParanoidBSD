module;
#include <cstdint>

export module pbsd.pkg.bootstrap;

import pbsd.core;
import pbsd.pkg.repo;

/// Burst 12 — bootstrap repo discovery and first-boot seeding.
export namespace pbsd::pkg::bootstrap {

inline constexpr char kDefaultRepoName[] = "pbsd-base";
inline constexpr char kDefaultRepoUrl[] = "https://pkg.pbsd.local/base";
inline constexpr char kSeedManifest[] = "bootstrap.pbsp";

struct BootstrapConfig {
    repo::RepoRef primary{};
    char manifest_path[128]{};
    bool offline{false};
};

[[nodiscard]] inline BootstrapConfig default_config() noexcept {
    BootstrapConfig cfg{};
    for (unsigned i = 0; kDefaultRepoName[i] != '\0' && i < 63; ++i) {
        cfg.primary.name[i] = kDefaultRepoName[i];
    }
    for (unsigned i = 0; kDefaultRepoUrl[i] != '\0' && i < 255; ++i) {
        cfg.primary.url[i] = kDefaultRepoUrl[i];
    }
    cfg.primary.flags = static_cast<unsigned>(repo::RepoFlag::Signed)
                      | static_cast<unsigned>(repo::RepoFlag::Immutable);
    for (unsigned i = 0; kSeedManifest[i] != '\0' && i < 127; ++i) {
        cfg.manifest_path[i] = kSeedManifest[i];
    }
    return cfg;
}

[[nodiscard]] inline Status validate_config(BootstrapConfig const& cfg) noexcept {
    if (repo::validate_repo(cfg.primary) != Status::Ok) {
        return Status::Invalid;
    }
    if (cfg.manifest_path[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::pkg::bootstrap
