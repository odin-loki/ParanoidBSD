module;
#include <cstdint>

export module pbsd.kernel.audit_trigger;

import pbsd.core;

/// Freestanding port of `security/audit/audit_trigger.c` one-byte trigger queue.
export namespace pbsd::kernel::audit_trigger {

inline constexpr unsigned kTriggerMax = 255;

struct TriggerInfo {
    unsigned char trigger{};
    unsigned      next_index{};
};

struct Queue {
    TriggerInfo entries[16]{};
    unsigned      head{};
    unsigned      tail{};
    unsigned      count{};
    bool          open{};
};

[[nodiscard]] inline Status validate_trigger(unsigned char trigger) noexcept {
    if (trigger == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status enqueue(Queue& q, unsigned char trigger) noexcept {
    if (validate_trigger(trigger) != Status::Ok) {
        return Status::Invalid;
    }
    if (q.count >= sizeof(q.entries) / sizeof(q.entries[0])) {
        return Status::NoMemory;
    }
    q.entries[q.tail].trigger = trigger;
    q.tail = (q.tail + 1) % (sizeof(q.entries) / sizeof(q.entries[0]));
    ++q.count;
    return Status::Ok;
}

[[nodiscard]] inline Status dequeue(Queue& q, unsigned char& trigger) noexcept {
    if (q.count == 0) {
        return Status::NotFound;
    }
    trigger = q.entries[q.head].trigger;
    q.head = (q.head + 1) % (sizeof(q.entries) / sizeof(q.entries[0]));
    --q.count;
    return Status::Ok;
}

[[nodiscard]] inline Status open(Queue& q) noexcept {
    if (q.open) {
        return Status::Busy;
    }
    q.open = true;
    return Status::Ok;
}

inline void close(Queue& q) noexcept {
    q.open = false;
    q.head = q.tail = q.count = 0;
}

} // namespace pbsd::kernel::audit_trigger
