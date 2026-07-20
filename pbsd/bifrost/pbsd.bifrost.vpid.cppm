module;
#include <cstdint>

export module pbsd.bifrost.vpid;

import pbsd.core;
import pbsd.bifrost.hypervisor;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/intel/vmcs.h — VPID/ASID tagging stubs.
export namespace pbsd::bifrost::vpid {

inline constexpr unsigned kMaxVpid = 65535;
inline constexpr unsigned kReserved = 0;

enum class TagKind : unsigned char {
    Vpid = 0,
    Asid = 1,
};

[[nodiscard]] inline Status validate_tag(unsigned tag, TagKind kind) noexcept {
    if (tag == kReserved || tag > kMaxVpid) {
        return Status::Invalid;
    }
    (void)kind;
    return Status::Ok;
}

[[nodiscard]] inline hypervisor::Backend backend_for(TagKind k) noexcept {
    return k == TagKind::Vpid ? hypervisor::Backend::Vmx : hypervisor::Backend::Svm;
}

} // namespace pbsd::bifrost::vpid
