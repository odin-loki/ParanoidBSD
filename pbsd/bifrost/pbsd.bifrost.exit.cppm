module;
#include <cstdint>

export module pbsd.bifrost.exit;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/include/vmm.h — VM exit reason codes.
export namespace pbsd::bifrost::exit {

enum class Code : unsigned int {
    Inout = 0,
    Vmx = 1,
    Bogus = 2,
    Rdmsr = 3,
    Wrmsr = 4,
    Hlt = 5,
    Mtrap = 6,
    Pause = 7,
    Paging = 8,
    InstEmul = 9,
    SpinupAp = 10,
    Rendezvous = 12,
    IoapicEoi = 13,
    Suspended = 14,
    InoutStr = 15,
    TaskSwitch = 16,
    Monitor = 17,
    Mwait = 18,
    Svm = 19,
    Reqidle = 20,
    Debug = 21,
    Vminsn = 22,
    Bpt = 23,
    Ipi = 24,
};

[[nodiscard]] inline Status validate_code(Code c) noexcept {
    switch (c) {
    case Code::Inout:
    case Code::Vmx:
    case Code::Svm:
    case Code::Paging:
    case Code::Hlt:
        return Status::Ok;
    default:
        return Status::Ok;
    }
}

[[nodiscard]] inline constexpr bool is_nested(Code c) noexcept {
    return c == Code::Vmx || c == Code::Svm;
}

} // namespace pbsd::bifrost::exit
