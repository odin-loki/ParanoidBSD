module;
#include <cstdint>

export module pbsd.bifrost.hypervisor;

import pbsd.core;

/// PROVENANCE: hbsd amd64/vmm/intel/vmcs.h, amd/vmcb.h, amd64/include/vmm.h, arm64/include/vmm.h
export namespace pbsd::bifrost::hypervisor {

enum class Backend : unsigned char { Vmx = 0, Svm = 1, VirtioMmio = 2 };

enum class ExitAction : unsigned char {
    Continue = 0,
    Inject   = 1,
    Halt     = 2,
    Teardown = 3,
};

/// amd64 enum vm_exitcode (vmm.h).
enum class Amd64VmExit : unsigned int {
    Inout       = 0,
    Vmx         = 1,
    Bogus       = 2,
    Rdmsr       = 3,
    Wrmsr       = 4,
    Hlt         = 5,
    Mtrap       = 6,
    Pause       = 7,
    Paging      = 8,
    InstEmul    = 9,
    Suspended   = 13,
    InoutStr    = 14,
    Svm         = 19,
    Debug       = 21,
    Bpt         = 23,
};

/// Intel VMX basic exit reasons (vmcs.h EXIT_REASON_*).
enum class VmxExit : unsigned int {
    Exception       = 0,
    ExtIntr         = 1,
    TripleFault     = 2,
    IntrWindow      = 7,
    TaskSwitch      = 9,
    Cpuid           = 10,
    Hlt             = 12,
    Invlpg          = 14,
    Vmcall          = 18,
    Inout           = 30,
    Rdmsr           = 31,
    Wrmsr           = 32,
    Mwait           = 36,
    Monitor         = 39,
    Pause           = 40,
    EptFault        = 48,
    EptMisconfig    = 49,
    VmxPreempt      = 52,
    Xsetbv          = 55,
    ApicWrite       = 56,
};

/// AMD SVM VMCB exit codes (vmcb.h VMCB_EXIT_*).
enum class VmcbExit : unsigned int {
    Intr        = 0x60,
    Nmi         = 0x61,
    Vintr       = 0x64,
    Cpuid       = 0x72,
    Hlt         = 0x78,
    Io          = 0x7B,
    Msr         = 0x7C,
    Vmmcall     = 0x81,
    Npf         = 0x400,
    Invalid     = 0xFFFFFFFFu,
};

/// VMCB intercept bits (vmcb.h VMCB_INTCPT_*).
enum class VmcbIntercept : unsigned long long {
    Intr        = 1ull << 0,
    Nmi         = 1ull << 1,
    Cpuid       = 1ull << 18,
    Hlt         = 1ull << 24,
    Io          = 1ull << 27,
    Msr         = 1ull << 28,
    Vmmcall     = 1ull << 33,
};

struct ExitMapping {
    Backend     backend{};
    unsigned    code{};
    Amd64VmExit unified{};
    ExitAction  action{};
    const char* name{};
};

inline constexpr ExitMapping kExitTable[] = {
    {Backend::Vmx, static_cast<unsigned>(VmxExit::Cpuid), Amd64VmExit::InstEmul, ExitAction::Continue, "vmx_cpuid"},
    {Backend::Vmx, static_cast<unsigned>(VmxExit::Inout), Amd64VmExit::Inout, ExitAction::Continue, "vmx_inout"},
    {Backend::Vmx, static_cast<unsigned>(VmxExit::Rdmsr), Amd64VmExit::Rdmsr, ExitAction::Continue, "vmx_rdmsr"},
    {Backend::Vmx, static_cast<unsigned>(VmxExit::Wrmsr), Amd64VmExit::Wrmsr, ExitAction::Continue, "vmx_wrmsr"},
    {Backend::Vmx, static_cast<unsigned>(VmxExit::EptFault), Amd64VmExit::Paging, ExitAction::Inject, "vmx_ept_fault"},
    {Backend::Vmx, static_cast<unsigned>(VmxExit::Hlt), Amd64VmExit::Hlt, ExitAction::Halt, "vmx_hlt"},
    {Backend::Vmx, static_cast<unsigned>(VmxExit::Vmcall), Amd64VmExit::InstEmul, ExitAction::Continue, "vmx_vmcall"},
    {Backend::Svm, static_cast<unsigned>(VmcbExit::Cpuid), Amd64VmExit::InstEmul, ExitAction::Continue, "svm_cpuid"},
    {Backend::Svm, static_cast<unsigned>(VmcbExit::Io), Amd64VmExit::Inout, ExitAction::Continue, "svm_io"},
    {Backend::Svm, static_cast<unsigned>(VmcbExit::Msr), Amd64VmExit::Rdmsr, ExitAction::Continue, "svm_msr"},
    {Backend::Svm, static_cast<unsigned>(VmcbExit::Npf), Amd64VmExit::Paging, ExitAction::Inject, "svm_npf"},
    {Backend::Svm, static_cast<unsigned>(VmcbExit::Vmmcall), Amd64VmExit::InstEmul, ExitAction::Continue, "svm_vmmcall"},
    {Backend::Svm, static_cast<unsigned>(VmcbExit::Hlt), Amd64VmExit::Hlt, ExitAction::Halt, "svm_hlt"},
    {Backend::VirtioMmio, 0, Amd64VmExit::Inout, ExitAction::Continue, "virtio_mmio_config"},
    {Backend::Vmx, static_cast<unsigned>(VmxExit::TripleFault), Amd64VmExit::Mtrap, ExitAction::Teardown, "vmx_triple_fault"},
    {Backend::Vmx, static_cast<unsigned>(VmxExit::Invlpg), Amd64VmExit::Paging, ExitAction::Continue, "vmx_invlpg"},
    {Backend::Vmx, static_cast<unsigned>(VmxExit::Mwait), Amd64VmExit::Pause, ExitAction::Continue, "vmx_mwait"},
    {Backend::Svm, static_cast<unsigned>(VmcbExit::Nmi), Amd64VmExit::Mtrap, ExitAction::Inject, "svm_nmi"},
    {Backend::Svm, static_cast<unsigned>(VmcbExit::Vintr), Amd64VmExit::Inout, ExitAction::Inject, "svm_vintr"},
};

[[nodiscard]] inline constexpr std::size_t exit_table_size() noexcept {
    return sizeof(kExitTable) / sizeof(kExitTable[0]);
}

[[nodiscard]] inline Status map_exit(Backend b, unsigned code,
                                     Amd64VmExit* out) noexcept {
    if (out == nullptr) {
        return Status::Invalid;
    }
    for (auto const& e : kExitTable) {
        if (e.backend == b && e.code == code) {
            *out = e.unified;
            return Status::Ok;
        }
    }
    return Status::Protocol;
}

[[nodiscard]] inline Status dispatch_exit(Backend b, unsigned code,
                                          Amd64VmExit* out,
                                          ExitAction* action) noexcept {
    if (out == nullptr || action == nullptr) {
        return Status::Invalid;
    }
    for (auto const& e : kExitTable) {
        if (e.backend == b && e.code == code) {
            *out = e.unified;
            *action = e.action;
            return Status::Ok;
        }
    }
    return Status::Protocol;
}

} // namespace pbsd::bifrost::hypervisor
