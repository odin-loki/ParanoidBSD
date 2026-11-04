module;
#include <cstdint>

export module pbsd.kernel.kern_membarrier;

export import pbsd.core;

/// Freestanding port of `kern/kern_membarrier.c` — membarrier helpers.
export namespace pbsd::kernel::kern_membarrier {

enum class Scope : unsigned char {
    Private = 0,
    Shared = 1,
};

enum class Kind : unsigned char {
    LoadLoad = 0,
    LoadStore = 1,
    StoreStore = 2,
    StoreLoad = 3,
};

[[nodiscard]] inline Status validate(Scope s, Kind k) noexcept {
    switch (s) {
    case Scope::Private:
    case Scope::Shared:
        break;
    default:
        return Status::Invalid;
    }
    switch (k) {
    case Kind::LoadLoad:
    case Kind::LoadStore:
    case Kind::StoreStore:
    case Kind::StoreLoad:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::kernel::kern_membarrier
