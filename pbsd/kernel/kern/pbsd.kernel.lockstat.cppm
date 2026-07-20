module;
#include <cstdint>

export module pbsd.kernel.lockstat;

import pbsd.core;

/// Freestanding port of `kern/kern_lockstat.c` SDT probe name constants.
export namespace pbsd::kernel::lockstat {

enum class ProbeKind : unsigned {
    AdaptiveAcquire,
    AdaptiveRelease,
    AdaptiveSpin,
    AdaptiveBlock,
    SpinAcquire,
    SpinRelease,
    SpinSpin,
    RwAcquire,
    RwRelease,
    RwBlock,
    RwSpin,
};

inline constexpr const char* probe_name(ProbeKind kind) noexcept {
    switch (kind) {
    case ProbeKind::AdaptiveAcquire: return "adaptive__acquire";
    case ProbeKind::AdaptiveRelease: return "adaptive__release";
    case ProbeKind::AdaptiveSpin:    return "adaptive__spin";
    case ProbeKind::AdaptiveBlock:   return "adaptive__block";
    case ProbeKind::SpinAcquire:     return "spin__acquire";
    case ProbeKind::SpinRelease:     return "spin__release";
    case ProbeKind::SpinSpin:        return "spin__spin";
    case ProbeKind::RwAcquire:       return "rw__acquire";
    case ProbeKind::RwRelease:       return "rw__release";
    case ProbeKind::RwBlock:         return "rw__block";
    case ProbeKind::RwSpin:          return "rw__spin";
    }
    return "unknown";
}

inline constexpr const char kProvider[] = "lockstat";

} // namespace pbsd::kernel::lockstat
