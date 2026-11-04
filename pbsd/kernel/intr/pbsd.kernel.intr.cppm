module;
#include <cstdint>

export module pbsd.kernel.intr;

export import pbsd.core;

/// Wave 4/5 — interrupt subsystem from sys/interrupt.h, kern/kern_intr.c.
export namespace pbsd::kernel::intr {

inline constexpr unsigned kIntrIrqMax = 256;

enum class IntrType : unsigned char {
    Unknown = 0,
    Edge = 1,
    Level = 2,
    Msi = 3,
    MsiX = 4,
};

enum class IntrState : unsigned char {
    Disabled = 0,
    Enabled = 1,
    Suspended = 2,
};

struct IntrResource {
    unsigned   irq{};
    IntrType   type{IntrType::Unknown};
    IntrState  state{IntrState::Disabled};
    unsigned   cpu_affinity{};
};

[[nodiscard]] constexpr Status validate_irq(unsigned irq) noexcept {
    if (irq >= kIntrIrqMax) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr bool is_msi(IntrType t) noexcept {
    return t == IntrType::Msi || t == IntrType::MsiX;
}

[[nodiscard]] constexpr bool can_enable(IntrState s) noexcept {
    return s == IntrState::Disabled || s == IntrState::Suspended;
}

[[nodiscard]] constexpr IntrState enable(IntrState s) noexcept {
    if (can_enable(s)) {
        return IntrState::Enabled;
    }
    return s;
}

} // namespace pbsd::kernel::intr
