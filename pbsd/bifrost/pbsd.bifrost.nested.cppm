module;
#include <cstdint>

export module pbsd.bifrost.nested;

import pbsd.core;
import pbsd.bifrost.hypervisor;

/// PROVENANCE: hbsd amd64/vmm — nested VMX/SVM capability stubs (Wave 8).
export namespace pbsd::bifrost::nested {

inline constexpr unsigned kMaxNestDepth = 3;

enum class NestLevel : unsigned char {
    L0 = 0,
    L1 = 1,
    L2 = 2,
};

enum class NestFeature : unsigned int {
    VmxSecondary = 1u << 0,
    Ept          = 1u << 1,
    Vpid         = 1u << 2,
    SvmNested    = 1u << 3,
    Npt          = 1u << 4,
};

struct NestConfig {
    NestLevel           level{NestLevel::L0};
    hypervisor::Backend backend{hypervisor::Backend::Vmx};
    unsigned int        features{0};
    std::uint64_t       shadow_ram_bytes{256ULL * 1024 * 1024};
};

struct NestStub {
    NestLevel           level{};
    hypervisor::Backend backend{};
    bool                attached{false};
    const char*         name{};
};

inline constexpr NestStub kNestStubTable[] = {
    {NestLevel::L1, hypervisor::Backend::Vmx, false, "vmx_l1_shadow"},
    {NestLevel::L2, hypervisor::Backend::Vmx, false, "vmx_l2_guest_hypervisor"},
    {NestLevel::L1, hypervisor::Backend::Svm, false, "svm_l1_nested"},
    {NestLevel::L2, hypervisor::Backend::Svm, false, "svm_l2_guest"},
};

[[nodiscard]] inline constexpr std::size_t nest_stub_table_size() noexcept {
    return sizeof(kNestStubTable) / sizeof(kNestStubTable[0]);
}

[[nodiscard]] inline Status validate_nest_depth(NestLevel level) noexcept {
    if (static_cast<unsigned>(level) >= kMaxNestDepth) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_nest_config(NestConfig const& cfg) noexcept {
    if (validate_nest_depth(cfg.level) != Status::Ok) {
        return Status::Invalid;
    }
    if (cfg.shadow_ram_bytes == 0) {
        return Status::Invalid;
    }
    if (cfg.level == NestLevel::L0 && cfg.features != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status attach_nested_stub(NestConfig const& cfg) noexcept {
    if (validate_nest_config(cfg) != Status::Ok) {
        return Status::Invalid;
    }
    if (cfg.level == NestLevel::L0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status dispatch_nested_exit(NestLevel level,
                                                 hypervisor::Backend b,
                                                 unsigned code,
                                                 hypervisor::Amd64VmExit* out) noexcept {
    if (validate_nest_depth(level) != Status::Ok || out == nullptr) {
        return Status::Invalid;
    }
    return hypervisor::map_exit(b, code, out);
}

} // namespace pbsd::bifrost::nested
