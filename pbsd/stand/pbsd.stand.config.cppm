module;

export module pbsd.stand.config;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/loader.conf
export namespace pbsd::stand::config {

inline constexpr const char kDefaultKernel[] = "/boot/kernel/kernel";
inline constexpr const char kDefaultRoot[] = "ufs:/";

struct LoaderConfig {
    const char* kernel{kDefaultKernel};
    const char* root{kDefaultRoot};
    bool verbose{false};
};

[[nodiscard]] inline Status validate(const LoaderConfig& cfg) noexcept {
    if (cfg.kernel == nullptr || cfg.root == nullptr) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::stand::config
