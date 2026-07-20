module;
#include <cstdint>

export module pbsd.bifrost.gpa_pool;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/vmm_mem.c
export namespace pbsd::bifrost::gpa_pool {

inline constexpr unsigned kMaxPages = 4096;

[[nodiscard]] inline Status validate_pages(unsigned pages) noexcept {
    return pages > 0 && pages <= kMaxPages ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::bifrost::gpa_pool
