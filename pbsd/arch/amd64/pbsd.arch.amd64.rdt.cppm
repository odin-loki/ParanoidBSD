module;
#include <cstdint>

export module pbsd.arch.amd64.rdt;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/x86/x86/rdt.c
export namespace pbsd::arch::amd64::rdt {

enum class Resource : unsigned char {
    L3Cache = 0,
    MemoryBandwidth = 1,
};

struct Domain {
    unsigned id{};
    Resource resource{Resource::L3Cache};
};

[[nodiscard]] inline Status validate_domain(const Domain& d) noexcept {
    return d.id <= 255 ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::arch::amd64::rdt
