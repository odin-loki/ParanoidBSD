module;
#include <cstdint>

export module pbsd.kernel.bsm_domain;

import pbsd.core;

/// Freestanding port of `security/audit/bsm_domain.c` — BSM protocol family mappings.
export namespace pbsd::kernel::bsm_domain {

inline constexpr int kNoLocalMapping = -600;
inline constexpr unsigned short kBsmPfUnspec = 0;
inline constexpr unsigned short kBsmPfLocal = 1;
inline constexpr unsigned short kBsmPfInet = 2;
inline constexpr unsigned short kBsmPfInet6 = 28;
inline constexpr int kPfUnspec = 0;
inline constexpr int kPfLocal = 1;
inline constexpr int kPfInet = 2;
inline constexpr int kPfInet6 = 28;

[[nodiscard]] inline int lookup_local(unsigned short bsm_domain) noexcept {
    switch (bsm_domain) {
    case kBsmPfUnspec: return kPfUnspec;
    case kBsmPfLocal: return kPfLocal;
    case kBsmPfInet: return kPfInet;
    case kBsmPfInet6: return kPfInet6;
    default: return kNoLocalMapping;
    }
}

[[nodiscard]] inline Status validate_domain(unsigned short bsm_domain) noexcept {
    return lookup_local(bsm_domain) != kNoLocalMapping ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::kernel::bsm_domain
