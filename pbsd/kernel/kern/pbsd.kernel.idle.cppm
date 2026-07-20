module;
#include <cstdint>

export module pbsd.kernel.idle;

import pbsd.core;

/// Freestanding port of `kern/kern_idle.c` per-CPU idle thread setup order.
export namespace pbsd::kernel::idle {

inline constexpr int kSiSubSchedIdle  = 0x00080000;
inline constexpr int kSiOrderFirst    = 0x00000000;

struct IdleCpu {
    int  cpu_id{};
    bool setup{};
};

[[nodiscard]] inline Status setup_cpu(IdleCpu& cpu) noexcept {
    if (cpu.cpu_id < 0) {
        return Status::Invalid;
    }
    if (cpu.setup) {
        return Status::Busy;
    }
    cpu.setup = true;
    return Status::Ok;
}

[[nodiscard]] inline Status validate_cpu_count(unsigned ncpus) noexcept {
    if (ncpus == 0 || ncpus > 4096) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::idle
