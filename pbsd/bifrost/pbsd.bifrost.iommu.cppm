module;
#include <cstdint>

export module pbsd.bifrost.iommu;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/io/iommu.c, intel/vtd.c, amd/amdviiommu.c
export namespace pbsd::bifrost::iommu {

enum class Backend : unsigned char {
    None = 0,
    IntelVtd = 1,
    AmdVi = 2,
};

enum class MapPerm : unsigned char {
    Read = 1,
    Write = 2,
    Execute = 4,
};

struct DomainConfig {
    Backend backend{Backend::None};
    std::uint32_t domain_id{0};
    bool passthrough{false};
};

[[nodiscard]] inline Status validate_domain(const DomainConfig& cfg) noexcept {
    if (cfg.backend == Backend::None && !cfg.passthrough) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned perm_mask(MapPerm p) noexcept {
    return static_cast<unsigned>(p);
}

} // namespace pbsd::bifrost::iommu
