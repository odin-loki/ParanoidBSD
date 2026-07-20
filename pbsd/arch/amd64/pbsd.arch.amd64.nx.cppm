module;
#include <cstdint>

export module pbsd.arch.amd64.nx;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/x86/x86/nx.c
export namespace pbsd::arch::amd64::nx {

inline constexpr unsigned kPageSize = 4096;

[[nodiscard]] inline Status validate_page(unsigned page) noexcept {
    return page >= kPageSize ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::arch::amd64::nx
