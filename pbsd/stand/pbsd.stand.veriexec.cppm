module;
#include <cstddef>

export module pbsd.stand.veriexec;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/veriexec.c
export namespace pbsd::stand::veriexec {

inline constexpr const char kManifestSuffix[] = ".verified";

enum class Policy : unsigned char {
    None = 0,
    Enforce = 1,
    Monitor = 2,
};

[[nodiscard]] inline Status validate_policy(unsigned p) noexcept {
    return p <= static_cast<unsigned>(Policy::Monitor) ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline Status validate_hash_length(std::size_t len) noexcept {
    if (len != 64 && len != 128) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::stand::veriexec
