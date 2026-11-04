module;
#include <cstdint>

export module pbsd.kernel.eventhandler;

export import pbsd.core;

/// Freestanding port of `sys/eventhandler.h` and `kern/subr_eventhandler.c`.
export namespace pbsd::kernel::eventhandler {

inline constexpr int kPriFirst = 0;
inline constexpr int kPriAny     = 10000;
inline constexpr int kPriLast  = 20000;
inline constexpr int kDeadPriority = -1;
inline constexpr unsigned kMaxEntries = 32;

enum class ShutdownPri : int {
    First   = kPriFirst,
    Default = kPriAny,
    Last    = kPriLast,
};

enum class DevEvent : unsigned char {
    DetachBegin,
    DetachComplete,
    DetachFailed,
};

using HandlerFn = void (*)(void*);

struct Entry {
    HandlerFn   func{nullptr};
    void*       arg{nullptr};
    int         priority{kPriAny};
    bool        dead{false};
    bool        in_use{false};
    Entry*      next{nullptr};
};

struct ListStub {
    const char* name{nullptr};
    Entry*      head{nullptr};
    Entry*      tail{nullptr};
    Entry       pool[kMaxEntries]{};
    unsigned    runcount{0};
    unsigned    deadcount{0};
    bool        locked{false};
};

[[nodiscard]] constexpr Status validate_priority(int pri) noexcept {
    if (pri == kDeadPriority) {
        return Status::Invalid;
    }
    if (pri < kPriFirst || pri > kPriLast) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Entry* alloc_entry(ListStub& list) noexcept {
    for (auto& slot : list.pool) {
        if (!slot.in_use) {
            slot.in_use = true;
            slot.dead = false;
            slot.next = nullptr;
            return &slot;
        }
    }
    return nullptr;
}

[[nodiscard]] inline Status register_handler(ListStub& list, HandlerFn func,
                                             void* arg, int priority) noexcept {
    if (list.name == nullptr || func == nullptr) {
        return Status::Invalid;
    }
    if (validate_priority(priority) != Status::Ok) {
        return Status::Invalid;
    }
    Entry* ep = alloc_entry(list);
    if (ep == nullptr) {
        return Status::NoMemory;
    }
    ep->func = func;
    ep->arg = arg;
    ep->priority = priority;
    if (list.head == nullptr) {
        list.head = ep;
        list.tail = ep;
        return Status::Ok;
    }
    Entry* prev = nullptr;
    for (Entry* cur = list.head; cur != nullptr; prev = cur, cur = cur->next) {
        if (!cur->dead && priority < cur->priority) {
            ep->next = cur;
            if (prev != nullptr) {
                prev->next = ep;
            } else {
                list.head = ep;
            }
            return Status::Ok;
        }
    }
    list.tail->next = ep;
    list.tail = ep;
    return Status::Ok;
}

[[nodiscard]] inline Status deregister(ListStub& list, Entry* tag) noexcept {
    if (tag == nullptr || !tag->in_use) {
        return Status::Invalid;
    }
    if (list.runcount > 0) {
        tag->dead = true;
        tag->priority = kDeadPriority;
        ++list.deadcount;
        return Status::Ok;
    }
    Entry* prev = nullptr;
    for (Entry* cur = list.head; cur != nullptr; prev = cur, cur = cur->next) {
        if (cur == tag) {
            if (prev != nullptr) {
                prev->next = cur->next;
            } else {
                list.head = cur->next;
            }
            if (list.tail == cur) {
                list.tail = prev;
            }
            cur->in_use = false;
            cur->func = nullptr;
            cur->arg = nullptr;
            cur->next = nullptr;
            cur->dead = false;
            return Status::Ok;
        }
    }
    return Status::NotFound;
}

[[nodiscard]] inline Status prune_list(ListStub& list) noexcept {
    Entry* prev = nullptr;
    Entry* cur = list.head;
    while (cur != nullptr) {
        Entry* next = cur->next;
        if (cur->dead) {
            if (prev != nullptr) {
                prev->next = next;
            } else {
                list.head = next;
            }
            if (list.tail == cur) {
                list.tail = prev;
            }
            cur->in_use = false;
            cur->func = nullptr;
            cur->arg = nullptr;
            cur->dead = false;
            cur->next = nullptr;
            --list.deadcount;
        } else {
            prev = cur;
        }
        cur = next;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status invoke(ListStub& list) noexcept {
    if (list.head == nullptr) {
        return Status::Ok;
    }
    ++list.runcount;
    for (Entry* cur = list.head; cur != nullptr; cur = cur->next) {
        if (cur->in_use && !cur->dead && cur->func != nullptr) {
            cur->func(cur->arg);
        }
    }
    --list.runcount;
    if (list.runcount == 0 && list.deadcount > 0) {
        return prune_list(list);
    }
    return Status::Ok;
}

struct NamedList {
    const char* name;
};

inline constexpr NamedList kStandardLists[] = {
    {"shutdown_pre_sync"},
    {"shutdown_post_sync"},
    {"shutdown_final"},
    {"power_resume"},
    {"power_suspend"},
    {"device_attach"},
    {"device_detach"},
    {"mountroot"},
};

[[nodiscard]] inline unsigned standard_list_count() noexcept {
    return static_cast<unsigned>(sizeof(kStandardLists) / sizeof(kStandardLists[0]));
}

} // namespace pbsd::kernel::eventhandler
