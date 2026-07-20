module;
#include <cstdint>

export module pbsd.bifrost.xsave;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/x86/include/fpu.h — XSAVE component layout.
export namespace pbsd::bifrost::xsave {

inline constexpr unsigned kHeaderOffset = 512;
inline constexpr unsigned kXcr0Offset = 464;
inline constexpr unsigned long long kCompEnabled = 1ull << 63;

enum class Component : unsigned long long {
    X87 = 1ull << 0,
    Sse = 1ull << 1,
    Avx = 1ull << 2,
    Mpx = 1ull << 3,
    Avx512 = 1ull << 5,
    Pt = 1ull << 8,
};

[[nodiscard]] inline Status validate_xstate_bv(unsigned long long bv) noexcept {
    if (bv == 0) {
        return Status::Invalid;
    }
    if ((bv & ~0x1FFFFFull) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool has_component(unsigned long long bv, Component c) noexcept {
    return (bv & static_cast<unsigned long long>(c)) != 0;
}

} // namespace pbsd::bifrost::xsave
