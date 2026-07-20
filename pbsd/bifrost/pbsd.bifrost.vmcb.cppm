module;
#include <cstdint>

export module pbsd.bifrost.vmcb;

import pbsd.core;
import pbsd.bifrost.hypervisor;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/amd/vmcb.h — VMCB intercept bits.
export namespace pbsd::bifrost::vmcb {

inline constexpr unsigned kCrIntcpt  = 0;
inline constexpr unsigned kDrIntcpt  = 1;
inline constexpr unsigned kExcIntcpt = 2;
inline constexpr unsigned kCtrl1Intcpt = 3;
inline constexpr unsigned kCtrl2Intcpt = 4;

enum class Ctrl1 : unsigned int {
    Intr        = 1u << 0,
    Nmi         = 1u << 1,
    Smi         = 1u << 2,
    Init        = 1u << 3,
    Vintr       = 1u << 4,
    Cr0Write    = 1u << 5,
    Rdtsc       = 1u << 14,
    Rdpmc       = 1u << 15,
    Cpuid       = 1u << 18,
    Hlt         = 1u << 24,
    Invlpg      = 1u << 25,
    Io          = 1u << 27,
    Msr         = 1u << 28,
    TaskSwitch  = 1u << 29,
};

enum class Ctrl2 : unsigned int {
    Vmrun  = 1u << 0,
    Vmmcall = 1u << 1,
    Vmload = 1u << 2,
    Vmsave = 1u << 3,
    Stgi   = 1u << 4,
    Clgi   = 1u << 5,
    Skinit = 1u << 6,
    Rdtscp = 1u << 7,
    Icebp  = 1u << 8,
    Wbinvd = 1u << 9,
    Monitor = 1u << 10,
    Mwait  = 1u << 11,
    Xsetbv = 1u << 13,
    Rdpru  = 1u << 14,
    EferWrite = 1u << 15,
    CrWrite = 1u << 16,
    CrRead  = 1u << 17,
    Invlpga = 1u << 18,
    Iopm    = 1u << 19,
    Msrpm   = 1u << 20,
    TaskSwitch = 1u << 21,
    FerrFreeze = 1u << 22,
    Shutdown   = 1u << 23,
};

[[nodiscard]] inline Status validate_intercept(unsigned ctrl1, unsigned ctrl2) noexcept {
    if ((ctrl1 & static_cast<unsigned>(Ctrl1::Cpuid))
        && (ctrl2 & static_cast<unsigned>(Ctrl2::Vmrun))) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline hypervisor::Backend backend_for_vmcb() noexcept {
    return hypervisor::Backend::Svm;
}

} // namespace pbsd::bifrost::vmcb
