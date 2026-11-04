module;
#include <cstdint>

export module pbsd.kernel.cpuset;

export import pbsd.core;

/// Wave 4/5 — CPU sets from sys/cpuset.h, kern/kern_cpuset.c.
export namespace pbsd::kernel::cpuset {

inline constexpr unsigned kCpuSetSize = 256;
inline constexpr unsigned kCpuSetWords = kCpuSetSize / (8 * sizeof(unsigned));

struct CpuSet {
    unsigned bits[kCpuSetWords]{};
};

[[nodiscard]] constexpr Status validate_cpu_id(unsigned cpu) noexcept {
    if (cpu >= kCpuSetSize) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr void set_cpu(CpuSet& set, unsigned cpu) noexcept {
    if (cpu < kCpuSetSize) {
        set.bits[cpu / (8 * sizeof(unsigned))] |= 1u << (cpu % (8 * sizeof(unsigned)));
    }
}

[[nodiscard]] constexpr void clear_cpu(CpuSet& set, unsigned cpu) noexcept {
    if (cpu < kCpuSetSize) {
        set.bits[cpu / (8 * sizeof(unsigned))] &= ~(1u << (cpu % (8 * sizeof(unsigned))));
    }
}

[[nodiscard]] constexpr bool is_set(const CpuSet& set, unsigned cpu) noexcept {
    if (cpu >= kCpuSetSize) {
        return false;
    }
    return (set.bits[cpu / (8 * sizeof(unsigned))] &
            (1u << (cpu % (8 * sizeof(unsigned))))) != 0;
}

[[nodiscard]] constexpr unsigned count_cpus(const CpuSet& set) noexcept {
    unsigned n = 0;
    for (unsigned i = 0; i < kCpuSetSize; ++i) {
        if (is_set(set, i)) {
            ++n;
        }
    }
    return n;
}

[[nodiscard]] constexpr Status validate_non_empty(const CpuSet& set) noexcept {
    return count_cpus(set) > 0 ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::kernel::cpuset
