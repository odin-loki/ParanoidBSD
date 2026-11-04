module;
#include <cstdint>

export module pbsd.geom.event;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/geom_event.c — GEOM topology event queue.
export namespace pbsd::geom::event {

enum class Kind : unsigned char {
    Create = 0,
    Destroy = 1,
    Taste = 2,
    Orphan = 3,
};

struct Event {
    Kind kind{Kind::Create};
    std::uint64_t provider_id{};
    bool pending{false};
};

inline constexpr unsigned kQueueDepth = 64;

struct Queue {
    Event slots[kQueueDepth]{};
    unsigned head{};
    unsigned tail{};
    unsigned size{};
};

[[nodiscard]] inline Status enqueue(Queue& q, Event e) noexcept {
    if (q.size >= kQueueDepth) {
        return Status::Busy;
    }
    e.pending = true;
    q.slots[q.tail] = e;
    q.tail = (q.tail + 1) % kQueueDepth;
    ++q.size;
    return Status::Ok;
}

[[nodiscard]] inline Status dequeue(Queue& q, Event& out) noexcept {
    if (q.size == 0) {
        return Status::NotFound;
    }
    out = q.slots[q.head];
    q.slots[q.head] = Event{};
    q.head = (q.head + 1) % kQueueDepth;
    --q.size;
    return Status::Ok;
}

} // namespace pbsd::geom::event
