module;
#include <cstdint>

export module pbsd.arch.amd64.tsx;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/x86/x86/tsx.c
export namespace pbsd::arch::amd64::tsx {

enum class Feature : unsigned char {
    Disabled = 0,
    Rtm = 1,
    Hle = 2,
};

[[nodiscard]] inline Status validate_feature(unsigned f) noexcept {
    return f <= static_cast<unsigned>(Feature::Hle) ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::arch::amd64::tsx
