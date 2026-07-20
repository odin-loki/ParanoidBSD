module;
#include <cstdint>

export module pbsd.arch.amd64.exception;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/x86/include/trap.h — x86 exception vectors.
export namespace pbsd::arch::amd64::exception {

enum class Vector : unsigned char {
    DivideError = 0,
    Debug = 1,
    Nmi = 2,
    Breakpoint = 3,
    Overflow = 4,
    Bounds = 5,
    InvalidOpcode = 6,
    DeviceNotAvailable = 7,
    DoubleFault = 8,
    InvalidTss = 10,
    SegmentNotPresent = 11,
    StackFault = 12,
    GeneralProtection = 13,
    PageFault = 14,
    X87Fp = 16,
    AlignmentCheck = 17,
    MachineCheck = 18,
    Simd = 19,
};

[[nodiscard]] inline Status validate_vector(Vector v) noexcept {
    switch (v) {
    case Vector::DivideError:
    case Vector::Debug:
    case Vector::Nmi:
    case Vector::Breakpoint:
    case Vector::PageFault:
    case Vector::GeneralProtection:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline bool is_fault(Vector v) noexcept {
    return v == Vector::PageFault || v == Vector::GeneralProtection
        || v == Vector::StackFault || v == Vector::SegmentNotPresent;
}

} // namespace pbsd::arch::amd64::exception
