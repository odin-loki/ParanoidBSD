module;
#include <cstdint>

export module pbsd.arch.arm64.ras;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/arm64/arm64/ras.c
export namespace pbsd::arch::arm64::ras {

enum class ErrorType : unsigned char {
    None = 0,
    Corrected = 1,
    Uncorrected = 2,
    Deferred = 3,
};

struct ErrorRecord {
    ErrorType type{ErrorType::None};
    std::uint64_t address{};
};

[[nodiscard]] inline Status validate_record(const ErrorRecord& r) noexcept {
    if (r.type == ErrorType::None && r.address != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::arch::arm64::ras
