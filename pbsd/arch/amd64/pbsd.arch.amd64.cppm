module;
#include <cstddef>

export module pbsd.arch.amd64;

import pbsd.core;
export import pbsd.arch.amd64.msr;
export import pbsd.arch.amd64.cpufunc;
export import pbsd.arch.amd64.pmap;
export import pbsd.arch.amd64.apic;
export import pbsd.arch.amd64.exception;
export import pbsd.arch.amd64.cr;
export import pbsd.arch.amd64.gdt;
export import pbsd.arch.amd64.idt;
export import pbsd.arch.amd64.tss;
export import pbsd.arch.amd64.fpu;
export import pbsd.arch.amd64.ident;
export import pbsd.arch.amd64.pic;
export import pbsd.arch.amd64.lapic;
export import pbsd.arch.amd64.segments;
export import pbsd.arch.amd64.pte;
export import pbsd.arch.amd64.rdrand;
export import pbsd.arch.amd64.mtrr;
export import pbsd.arch.amd64.smp;
export import pbsd.arch.amd64.pat;
export import pbsd.arch.amd64.nx;
export import pbsd.arch.amd64.avx;
export import pbsd.arch.amd64.svm;
export import pbsd.arch.amd64.vmx;
export import pbsd.arch.amd64.acpi;
export import pbsd.arch.amd64.ioapic;
export import pbsd.arch.amd64.nmi;
export import pbsd.arch.amd64.tsx;
export import pbsd.arch.amd64.smap;
export import pbsd.arch.amd64.cpuid;
export import pbsd.arch.amd64.x2apic;
export import pbsd.arch.amd64.rdt;
export import pbsd.arch.amd64.amd;

/// Wave 7 — amd64 arch stubs
/// PROVENANCE: hbsd/src/sys/x86/include/specialreg.h, amd64/vmm/intel/vmcs.h
export namespace pbsd::arch::amd64 {

inline constexpr std::size_t kPageShift = 12;
inline constexpr std::size_t kPageSize  = 1ull << kPageShift;
inline constexpr std::size_t kStackAlign = 16;

struct RegisterFrame {
    unsigned long long rax{};
    unsigned long long rbx{};
    unsigned long long rcx{};
    unsigned long long rdx{};
    unsigned long long rsi{};
    unsigned long long rdi{};
    unsigned long long rbp{};
    unsigned long long r8{};
    unsigned long long r9{};
    unsigned long long r10{};
    unsigned long long r11{};
    unsigned long long r12{};
    unsigned long long r13{};
    unsigned long long r14{};
    unsigned long long r15{};
    unsigned long long rip{};
    unsigned long long rflags{};
    unsigned long long rsp{};
};

enum class CpuFeature : unsigned int {
    None = 0,
    Smap = 1u << 0,
    Smep = 1u << 1,
    Umip = 1u << 2,
    Vmx  = 1u << 3,
    Svm  = 1u << 4,
};

[[nodiscard]] constexpr CpuFeature operator|(CpuFeature a, CpuFeature b) noexcept {
    return static_cast<CpuFeature>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}

[[nodiscard]] inline Status validate_frame(const RegisterFrame& frame) noexcept {
    if (frame.rsp % kStackAlign != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_long_mode(unsigned long long efer) noexcept {
    return msr::validate_efer(efer);
}

class ArchProfile {
public:
    [[nodiscard]] constexpr std::size_t page_size() const noexcept { return kPageSize; }
    [[nodiscard]] constexpr CpuFeature features() const noexcept { return features_; }
    [[nodiscard]] constexpr unsigned long long default_efer() const noexcept {
        return static_cast<unsigned long long>(msr::EferFlag::Sce)
             | static_cast<unsigned long long>(msr::EferFlag::Lme)
             | static_cast<unsigned long long>(msr::EferFlag::Nxe);
    }

private:
    CpuFeature features_{CpuFeature::Smap | CpuFeature::Smep | CpuFeature::Vmx};
};

} // namespace pbsd::arch::amd64
