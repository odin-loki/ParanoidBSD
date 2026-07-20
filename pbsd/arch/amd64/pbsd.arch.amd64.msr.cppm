module;
#include <cstdint>

export module pbsd.arch.amd64.msr;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/x86/include/specialreg.h, amd64/vmm/intel/vmcs.h
export namespace pbsd::arch::amd64::msr {

inline constexpr unsigned kMsrEfer      = 0xc0000080;
inline constexpr unsigned kMsrStar      = 0xc0000081;
inline constexpr unsigned kMsrLstar     = 0xc0000082;
inline constexpr unsigned kMsrCstar     = 0xc0000083;
inline constexpr unsigned kMsrSfMask    = 0xc0000084;
inline constexpr unsigned kMsrFsBase    = 0xc0000100;
inline constexpr unsigned kMsrGsBase    = 0xc0000101;
inline constexpr unsigned kMsrKgBase    = 0xc0000102;
inline constexpr unsigned kMsrApicBase  = 0x01b;
inline constexpr unsigned kMsrIa32SpecCtrl = 0x048;
inline constexpr unsigned kMsrIa32PredCtrl = 0x049;
inline constexpr unsigned kMsrVmCr      = 0xc0010114;
inline constexpr unsigned kMsrVmHsavePa = 0xc0010117;
inline constexpr unsigned kMsrTscAux    = 0xc0000103;
inline constexpr unsigned kMsrIa32FeatCtrl = 0x03a;
inline constexpr unsigned kMsrIa32VmxBasic = 0x480;
inline constexpr unsigned kMsrIa32VmxPinbased = 0x481;
inline constexpr unsigned kMsrIa32VmxProcbased = 0x482;
inline constexpr unsigned kMsrIa32VmxExit = 0x483;
inline constexpr unsigned kMsrIa32VmxEntry = 0x484;
inline constexpr unsigned kMsrIa32VmxMisc = 0x485;
inline constexpr unsigned kMsrIa32VmxCr0Fixed0 = 0x486;
inline constexpr unsigned kMsrIa32VmxCr0Fixed1 = 0x487;
inline constexpr unsigned kMsrIa32VmxCr4Fixed0 = 0x488;
inline constexpr unsigned kMsrIa32VmxCr4Fixed1 = 0x489;
inline constexpr unsigned kMsrTsc             = 0x010;
inline constexpr unsigned kMsrPlatformInfo    = 0x0ce;
inline constexpr unsigned kMsrMperf           = 0x0e7;
inline constexpr unsigned kMsrAperf           = 0x0e8;
inline constexpr unsigned kMsrMtrrCap         = 0x0fe;
inline constexpr unsigned kMsrIa32ArchCap     = 0x10a;
inline constexpr unsigned kMsrSysenterCs      = 0x174;
inline constexpr unsigned kMsrSysenterEsp     = 0x175;
inline constexpr unsigned kMsrSysenterEip     = 0x176;
inline constexpr unsigned kMsrMcgCap          = 0x179;
inline constexpr unsigned kMsrMcgStatus       = 0x17a;
inline constexpr unsigned kMsrIa32MiscEnable  = 0x1a0;
inline constexpr unsigned kMsrIa32Pat         = 0x277;
inline constexpr unsigned kMsrIa32DebugCtl    = 0x1d9;
inline constexpr unsigned kMsrIa32PerfGlobalCtrl = 0x38f;
inline constexpr unsigned kMsrIa32UcodeRev    = 0x08b;
inline constexpr unsigned kMsrIa32Smbase     = 0x09e;
inline constexpr unsigned kMsrIa32SmrrPhysbase = 0x1f2;
inline constexpr unsigned kMsrIa32SmrrPhysmask = 0x1f3;
inline constexpr unsigned kMsrIa32Pkrs       = 0x6e1;

enum class Cr4Flag : unsigned long long {
    Vme     = 1ull << 0,
    Pse     = 1ull << 4,
    Pae     = 1ull << 5,
    Mce     = 1ull << 6,
    Pge     = 1ull << 7,
    OsFxsr  = 1ull << 9,
    OsXmm   = 1ull << 10,
    VmxE    = 1ull << 13,
    Smep    = 1ull << 20,
    Smap    = 1ull << 21,
    Pke     = 1ull << 22,
    Cet     = 1ull << 23,
};

enum class EferFlag : unsigned long long {
    Sce  = 0x001,
    Lme  = 0x100,
    Lma  = 0x400,
    Nxe  = 0x800,
    Svm  = 0x1000,
    Ffxsr = 0x4000,
};

enum class VmCrFlag : unsigned long long {
    SvmDis = 1ull << 4,
    Lock   = 1ull << 3,
};

/// VMCS field encodings (Appendix H, Intel SDM Vol3B).
inline constexpr unsigned kVmcsExitReason         = 0x00004402;
inline constexpr unsigned kVmcsGuestRip           = 0x0000681e;
inline constexpr unsigned kVmcsGuestRsp           = 0x0000681c;
inline constexpr unsigned kVmcsGuestCr3           = 0x00006802;
inline constexpr unsigned kVmcsExitQualification  = 0x00006400;
inline constexpr unsigned kVmcsGuestPhysicalAddr  = 0x00002400;
inline constexpr unsigned kVmcsGuestLinearAddr    = 0x0000640a;
inline constexpr unsigned kVmcsEptp               = 0x0000201a;
inline constexpr unsigned kVmcsPinBasedExecCtrl   = 0x00004000;
inline constexpr unsigned kVmcsProcBasedExecCtrl  = 0x00004002;
inline constexpr unsigned kVmcsProcBasedExecCtrl2 = 0x0000401e;

enum class VmxExitReason : unsigned int {
    Exception       = 0,
    ExtIntr         = 1,
    TripleFault     = 2,
    Init            = 3,
    Sipi            = 4,
    IntrWindow      = 7,
    TaskSwitch      = 9,
    Cpuid           = 10,
    Vmcall          = 18,
    Hlt             = 12,
    Invlpg          = 14,
    Rdmsr           = 31,
    Wrmsr           = 32,
    Inout           = 30,
    Mwait           = 36,
    Monitor         = 39,
    Pause           = 40,
    EptFault        = 48,
    EptMisconfig    = 49,
    VmxPreempt      = 52,
    InvVpid         = 53,
    Wbinvd          = 54,
    Xsetbv          = 55,
    ApicWrite       = 56,
    InvPcid         = 58,
    VmFunc          = 59,
    Xsaves          = 63,
    Xrstors         = 64,
};

inline constexpr VmxExitReason kGuestExitReasons[] = {
    VmxExitReason::Cpuid,
    VmxExitReason::Hlt,
    VmxExitReason::Inout,
    VmxExitReason::Rdmsr,
    VmxExitReason::Wrmsr,
    VmxExitReason::EptFault,
    VmxExitReason::EptMisconfig,
    VmxExitReason::Vmcall,
    VmxExitReason::Xsetbv,
    VmxExitReason::Invlpg,
    VmxExitReason::TripleFault,
    VmxExitReason::TaskSwitch,
    VmxExitReason::Mwait,
    VmxExitReason::Monitor,
    VmxExitReason::Pause,
    VmxExitReason::VmxPreempt,
    VmxExitReason::Xsaves,
    VmxExitReason::Xrstors,
};

[[nodiscard]] inline constexpr std::size_t guest_exit_reason_count() noexcept {
    return sizeof(kGuestExitReasons) / sizeof(kGuestExitReasons[0]);
}

[[nodiscard]] inline Status validate_cr4_vmx(unsigned long long cr4) noexcept {
    auto flags = static_cast<Cr4Flag>(cr4);
    if ((static_cast<unsigned long long>(flags) & static_cast<unsigned long long>(Cr4Flag::VmxE)) != 0
        && (static_cast<unsigned long long>(flags) & static_cast<unsigned long long>(Cr4Flag::Pae)) == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline constexpr bool efer_has(EferFlag set, EferFlag bit) noexcept {
    return (static_cast<unsigned long long>(set) & static_cast<unsigned long long>(bit)) != 0;
}

[[nodiscard]] inline Status validate_efer(unsigned long long val) noexcept {
    auto flags = static_cast<EferFlag>(val);
    if (efer_has(flags, EferFlag::Lma) && !efer_has(flags, EferFlag::Lme)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

struct MsrEntry {
    unsigned    addr{};
    const char* name{};
};

inline constexpr MsrEntry kMsrTable[] = {
    {kMsrEfer, "EFER"},
    {kMsrTsc, "TSC"},
    {kMsrApicBase, "APIC_BASE"},
    {kMsrIa32FeatCtrl, "IA32_FEATURE_CONTROL"},
    {kMsrIa32SpecCtrl, "IA32_SPEC_CTRL"},
    {kMsrIa32VmxBasic, "IA32_VMX_BASIC"},
    {kMsrIa32VmxPinbased, "IA32_VMX_PINBASED"},
    {kMsrIa32VmxProcbased, "IA32_VMX_PROCBASED"},
    {kMsrIa32VmxExit, "IA32_VMX_EXIT"},
    {kMsrIa32VmxEntry, "IA32_VMX_ENTRY"},
    {kMsrPlatformInfo, "PLATFORM_INFO"},
    {kMsrMtrrCap, "MTRRcap"},
    {kMsrSysenterCs, "SYSENTER_CS"},
    {kMsrMcgCap, "MCG_CAP"},
    {kMsrIa32Pat, "IA32_PAT"},
    {kMsrVmCr, "VM_CR"},
    {kMsrVmHsavePa, "VM_HSAVE_PA"},
};

[[nodiscard]] inline constexpr std::size_t msr_table_size() noexcept {
    return sizeof(kMsrTable) / sizeof(kMsrTable[0]);
}

[[nodiscard]] inline constexpr bool is_vmx_msr(unsigned addr) noexcept {
    return addr >= kMsrIa32VmxBasic && addr <= kMsrIa32VmxCr4Fixed1;
}

[[nodiscard]] inline Status validate_msr_access(unsigned addr, bool wrmsr) noexcept {
    if (addr == kMsrIa32FeatCtrl && wrmsr) {
        return Status::Denied;
    }
    if (is_vmx_msr(addr) && !wrmsr) {
        return Status::Ok;
    }
    (void)wrmsr;
    return Status::Ok;
}

[[nodiscard]] inline constexpr bool is_guest_exit(VmxExitReason r) noexcept {
    switch (r) {
    case VmxExitReason::Cpuid:
    case VmxExitReason::Hlt:
    case VmxExitReason::Inout:
    case VmxExitReason::Rdmsr:
    case VmxExitReason::Wrmsr:
    case VmxExitReason::EptFault:
        return true;
    default:
        return false;
    }
}

} // namespace pbsd::arch::amd64::msr
