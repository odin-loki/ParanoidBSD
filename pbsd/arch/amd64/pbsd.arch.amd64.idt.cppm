module;
#include <cstdint>

export module pbsd.arch.amd64.idt;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/include/trap.h — IDT vector numbers.
export namespace pbsd::arch::amd64::idt {

inline constexpr unsigned kNumVectors = 256;
inline constexpr unsigned kNumExceptions = 32;
inline constexpr unsigned kIrqBase = 32;

enum class Exception : unsigned char {
    DivideError = 0,
    Debug = 1,
    Nmi = 2,
    Breakpoint = 3,
    Overflow = 4,
    Bound = 5,
    InvalidOpcode = 6,
    DeviceNotAvailable = 7,
    DoubleFault = 8,
    InvalidTss = 10,
    SegmentNotPresent = 11,
    StackFault = 12,
    GeneralProtection = 13,
    PageFault = 14,
    X87Fpu = 16,
    AlignmentCheck = 17,
    MachineCheck = 18,
    SimdFpu = 19,
};

[[nodiscard]] inline bool is_exception(unsigned vec) noexcept {
    return vec < kNumExceptions;
}

[[nodiscard]] inline bool is_irq(unsigned vec) noexcept {
    return vec >= kIrqBase && vec < kNumVectors;
}

[[nodiscard]] inline Status validate_vector(unsigned vec) noexcept {
    return vec < kNumVectors ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline unsigned irq_to_vector(unsigned irq) noexcept {
    return kIrqBase + irq;
}

} // namespace pbsd::arch::amd64::idt
