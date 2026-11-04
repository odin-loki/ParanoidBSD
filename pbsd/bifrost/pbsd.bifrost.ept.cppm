module;
#include <cstdint>

export module pbsd.bifrost.ept;

import pbsd.core;
import pbsd.bifrost.hypervisor;

/// PROVENANCE: hbsd amd64/vmm/intel/ept.h, amd64/vmm/amd/npt.h — nested paging stubs.
export namespace pbsd::bifrost::ept {

enum class EntryFlag : unsigned long long {
    Read        = 1ull << 0,
    Write       = 1ull << 1,
    Execute     = 1ull << 2,
    UserMode    = 1ull << 4,
    Accessed    = 1ull << 8,
    Dirty       = 1ull << 9,
    ExecuteUser = 1ull << 10,
    LargePage   = 1ull << 7,
};

enum class ViolationType : unsigned char {
    Read   = 0,
    Write  = 1,
    Execute = 2,
};

struct EptViolation {
    hypervisor::Backend backend{hypervisor::Backend::Vmx};
    std::uint64_t       guest_pa{};
    std::uint64_t       guest_va{};
    unsigned long long  entry_flags{};
    ViolationType       type{};
};

inline constexpr unsigned kPageShift = 12;
inline constexpr std::uint64_t kPageSize = 4096;

[[nodiscard]] inline std::uint64_t page_align(std::uint64_t addr) noexcept {
    return addr & ~(kPageSize - 1);
}

[[nodiscard]] inline bool entry_allows(EntryFlag have, EntryFlag need) noexcept {
    return (static_cast<unsigned long long>(have)
            & static_cast<unsigned long long>(need))
           == static_cast<unsigned long long>(need);
}

[[nodiscard]] inline Status validate_violation(EptViolation const& v) noexcept {
    if (v.guest_pa == 0) {
        return Status::Invalid;
    }
    if (v.backend != hypervisor::Backend::Vmx && v.backend != hypervisor::Backend::Svm) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status map_violation_to_exit(EptViolation const& v,
                                                  hypervisor::Amd64VmExit* out) noexcept {
    if (validate_violation(v) != Status::Ok || out == nullptr) {
        return Status::Invalid;
    }
    *out = hypervisor::Amd64VmExit::Paging;
    return Status::Ok;
}

} // namespace pbsd::bifrost::ept
