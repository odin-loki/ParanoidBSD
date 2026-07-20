module;
#include <cstdint>

export module pbsd.kernel.kern_shutdown;

export import pbsd.core;

/// Freestanding port of `kern/kern_shutdown.c` — shutdown helpers.
export namespace pbsd::kernel::kern_shutdown {

enum class Phase : unsigned char {
    Sync = 0,
    Files = 1,
    Final = 2,
};

[[nodiscard]] inline Status validate_phase(Phase p) noexcept {
    switch (p) {
    case Phase::Sync:
    case Phase::Files:
    case Phase::Final:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::kernel::kern_shutdown
