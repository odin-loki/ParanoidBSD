module;
#include <cstdint>

export module pbsd.kernel.numa;

export import pbsd.core;

/// Wave 4/5 — NUMA topology from sys/numa.h, vm/vm_numa.c.
export namespace pbsd::kernel::numa {

inline constexpr unsigned kNumaDomainMax = 64;
inline constexpr unsigned kNumaPolicyDefault = 0;
inline constexpr unsigned kNumaPolicyBind    = 1;
inline constexpr unsigned kNumaPolicyInterleave = 2;
inline constexpr unsigned kNumaPolicyPreferred  = 3;

struct NumaDomain {
    unsigned id{};
    unsigned cpu_count{};
    std::uint64_t mem_bytes{};
    bool active{};
};

struct NumaPolicy {
    unsigned policy{kNumaPolicyDefault};
    unsigned domain_id{};
};

[[nodiscard]] constexpr Status validate_domain(unsigned id, unsigned ndomains) noexcept {
    if (id >= ndomains || ndomains > kNumaDomainMax) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr Status validate_policy(unsigned policy) noexcept {
    if (policy > kNumaPolicyPreferred) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr bool policy_requires_domain(unsigned policy) noexcept {
    return policy == kNumaPolicyBind || policy == kNumaPolicyPreferred;
}

} // namespace pbsd::kernel::numa
