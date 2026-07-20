module;
#include <cstdint>

export module pbsd.kernel.refcnt;

export import pbsd.core;

/// Wave 4/5 — reference count helpers from sys/refcount.h.
export namespace pbsd::kernel::refcnt {

struct Refcount {
    std::uint32_t count{};
};

[[nodiscard]] inline Status acquire(Refcount& r) noexcept {
    if (r.count == ~0u) {
        return Status::Invalid;
    }
    ++r.count;
    return Status::Ok;
}

[[nodiscard]] inline Status release(Refcount& r) noexcept {
    if (r.count == 0) {
        return Status::Invalid;
    }
    --r.count;
    return Status::Ok;
}

[[nodiscard]] constexpr bool is_shared(const Refcount& r) noexcept {
    return r.count > 1;
}

} // namespace pbsd::kernel::refcnt
