module;
#include <cstdint>

export module pbsd.kernel.kern_switch;

export import pbsd.core;

/// Freestanding port of `kern/kern_switch.c` — switch helpers.
export namespace pbsd::kernel::kern_switch {

enum class Reason : unsigned char {
    Preempt = 0,
    Sleep = 1,
    Yield = 2,
    Exit = 3,
};

[[nodiscard]] inline Status validate_reason(Reason r) noexcept {
    switch (r) {
    case Reason::Preempt:
    case Reason::Sleep:
    case Reason::Yield:
    case Reason::Exit:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::kernel::kern_switch
