module;
#include <cstdint>

export module pbsd.arch.amd64.smap;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/x86/x86/smap.c
export namespace pbsd::arch::amd64::smap {

[[nodiscard]] inline bool smap_enabled(unsigned cr4) noexcept {
    return (cr4 & (1u << 21)) != 0;
}

} // namespace pbsd::arch::amd64::smap
