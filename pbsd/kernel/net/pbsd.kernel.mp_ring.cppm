module;
#include <cstdint>

export module pbsd.kernel.mp_ring;

import pbsd.core;

/// Freestanding port of `net/mp_ring.h` — multi-producer NIC enqueue ring stats.
export namespace pbsd::kernel::mp_ring {

inline constexpr unsigned kCacheLine = 64;

struct RingStats {
    std::uint64_t enqueues{};
    std::uint64_t drops{};
    std::uint64_t starts{};
    std::uint64_t stalls{};
    std::uint64_t restarts{};
    std::uint64_t abdications{};
};

struct RingStub {
    int           size{};
    void*         cookie{};
    RingStats     stats{};
    unsigned      head{};
    unsigned      tail{};
    bool          stalled{};
};

[[nodiscard]] inline Status validate_size(int size) noexcept {
    if (size <= 0 || (size & (size - 1)) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status init(RingStub& ring, int size, void* cookie) noexcept {
    if (validate_size(size) != Status::Ok) {
        return Status::Invalid;
    }
    ring.size = size;
    ring.cookie = cookie;
    ring.head = 0;
    ring.tail = 0;
    ring.stalled = false;
    ring.stats = {};
    return Status::Ok;
}

[[nodiscard]] inline unsigned capacity(const RingStub& ring) noexcept {
    if (ring.size <= 0) {
        return 0;
    }
    return static_cast<unsigned>(ring.size);
}

[[nodiscard]] inline unsigned occupancy(const RingStub& ring) noexcept {
    const unsigned cap = capacity(ring);
    if (cap == 0) {
        return 0;
    }
    return (ring.head + cap - ring.tail) % cap;
}

[[nodiscard]] inline bool is_full(const RingStub& ring) noexcept {
    return occupancy(ring) + 1 >= capacity(ring);
}

[[nodiscard]] inline bool is_idle(const RingStub& ring) noexcept {
    return ring.head == ring.tail;
}

[[nodiscard]] inline Status enqueue(RingStub& ring, unsigned count) noexcept {
    if (count == 0) {
        return Status::Invalid;
    }
    if (occupancy(ring) + count >= capacity(ring)) {
        ring.stats.drops += count;
        return Status::Denied;
    }
    ring.head = (ring.head + count) % capacity(ring);
    ring.stats.enqueues += count;
    return Status::Ok;
}

[[nodiscard]] inline unsigned drain(RingStub& ring, unsigned max) noexcept {
    if (is_idle(ring) || max == 0) {
        return 0;
    }
    const unsigned occ = occupancy(ring);
    const unsigned n = occ < max ? occ : max;
    ring.tail = (ring.tail + n) % capacity(ring);
    ++ring.stats.starts;
    return n;
}

[[nodiscard]] inline void reset_stats(RingStub& ring) noexcept {
    ring.stats = {};
}

[[nodiscard]] inline bool is_stalled(const RingStub& ring) noexcept {
    return ring.stalled;
}

[[nodiscard]] inline Status mark_stalled(RingStub& ring) noexcept {
    if (ring.stalled) {
        return Status::Busy;
    }
    ring.stalled = true;
    ++ring.stats.stalls;
    return Status::Ok;
}

[[nodiscard]] inline Status clear_stalled(RingStub& ring) noexcept {
    if (!ring.stalled) {
        return Status::Invalid;
    }
    ring.stalled = false;
    ++ring.stats.restarts;
    return Status::Ok;
}

} // namespace pbsd::kernel::mp_ring
