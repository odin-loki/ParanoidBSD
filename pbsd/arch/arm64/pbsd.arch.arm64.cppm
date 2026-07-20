module;
#include <cstddef>

export module pbsd.arch.arm64;

import pbsd.core;
export import pbsd.arch.arm64.sysreg;
export import pbsd.arch.arm64.exception;
export import pbsd.arch.arm64.pmap;
export import pbsd.arch.arm64.gic;
export import pbsd.arch.arm64.gicv3;
export import pbsd.arch.arm64.timer;
export import pbsd.arch.arm64.cpufunc;
export import pbsd.arch.arm64.fpu;
export import pbsd.arch.arm64.ident;
export import pbsd.arch.arm64.vmm;
export import pbsd.arch.arm64.smmu;
export import pbsd.arch.arm64.vfp;
export import pbsd.arch.arm64.dcz;
export import pbsd.arch.arm64.spinlock;
export import pbsd.arch.arm64.smp;
export import pbsd.arch.arm64.mmu;
export import pbsd.arch.arm64.pauth;
export import pbsd.arch.arm64.bti;
export import pbsd.arch.arm64.mte;
export import pbsd.arch.arm64.sve;
export import pbsd.arch.arm64.pac;
export import pbsd.arch.arm64.el2;
export import pbsd.arch.arm64.ras;
export import pbsd.arch.arm64.spectre;

/// Wave 7 — arm64 arch stubs (C++ only; asm stays in hbsd).
/// PROVENANCE: hbsd/src/sys/arm64/include/armreg.h, hypervisor.h
export namespace pbsd::arch::arm64 {

inline constexpr std::size_t kPageShift = 12;
inline constexpr std::size_t kPageSize  = 1ull << kPageShift;
inline constexpr std::size_t kStackAlign = 16;

struct RegisterFrame {
    unsigned long long x0{};
    unsigned long long x1{};
    unsigned long long x2{};
    unsigned long long x3{};
    unsigned long long x4{};
    unsigned long long x5{};
    unsigned long long x6{};
    unsigned long long x7{};
    unsigned long long x8{};
    unsigned long long x9{};
    unsigned long long x10{};
    unsigned long long x11{};
    unsigned long long x12{};
    unsigned long long x13{};
    unsigned long long x14{};
    unsigned long long x15{};
    unsigned long long x16{};
    unsigned long long x17{};
    unsigned long long x18{};
    unsigned long long x19{};
    unsigned long long x20{};
    unsigned long long x21{};
    unsigned long long x22{};
    unsigned long long x23{};
    unsigned long long x24{};
    unsigned long long x25{};
    unsigned long long x26{};
    unsigned long long x27{};
    unsigned long long x28{};
    unsigned long long x29{};
    unsigned long long x30{};
    unsigned long long sp{};
    unsigned long long pc{};
    unsigned long long pstate{};
};

enum class CpuFeature : unsigned int {
    None = 0,
    Pan  = 1u << 0,
    Uao  = 1u << 1,
    Vhe  = 1u << 2,
};

[[nodiscard]] constexpr CpuFeature operator|(CpuFeature a, CpuFeature b) noexcept {
    return static_cast<CpuFeature>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}

[[nodiscard]] inline Status validate_frame(const RegisterFrame& frame) noexcept {
    if (frame.sp % kStackAlign != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_sctlr(unsigned long long sctlr_el1) noexcept {
    return sysreg::validate_sctlr_el1(sctlr_el1);
}

class ArchProfile {
public:
    [[nodiscard]] constexpr std::size_t page_size() const noexcept { return kPageSize; }
    [[nodiscard]] constexpr CpuFeature features() const noexcept { return features_; }
    [[nodiscard]] constexpr unsigned long long guest_sctlr_reset() const noexcept {
        return sysreg::sctlr_el1_reset();
    }
    [[nodiscard]] constexpr unsigned long long host_sctlr_el2() const noexcept {
        return sysreg::sctlr_el2_host();
    }

private:
    static constexpr CpuFeature kDefaultFeatures =
        static_cast<CpuFeature>(static_cast<unsigned>(CpuFeature::Pan)
                                | static_cast<unsigned>(CpuFeature::Vhe));
    CpuFeature features_{kDefaultFeatures};
};

} // namespace pbsd::arch::arm64
