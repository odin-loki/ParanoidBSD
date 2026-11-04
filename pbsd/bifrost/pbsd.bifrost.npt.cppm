module;
#include <cstdint>

export module pbsd.bifrost.npt;

import pbsd.core;
import pbsd.bifrost.hypervisor;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/amd/npt.h — AMD NPT fault bits.
export namespace pbsd::bifrost::npt {

enum class FaultFlag : unsigned long long {
    Present  = 1ull << 0,
    Write    = 1ull << 1,
    User     = 1ull << 2,
    Fetch    = 1ull << 4,
    Final    = 1ull << 32,
};

struct Fault {
    unsigned long long gpa{};
    unsigned long long flags{};
};

[[nodiscard]] inline bool is_write(Fault const& f) noexcept {
    return (f.flags & static_cast<unsigned long long>(FaultFlag::Write)) != 0;
}

[[nodiscard]] inline Status validate_fault(Fault const& f) noexcept {
    if (f.gpa == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline hypervisor::Backend backend_for_npt() noexcept {
    return hypervisor::Backend::Svm;
}

} // namespace pbsd::bifrost::npt
