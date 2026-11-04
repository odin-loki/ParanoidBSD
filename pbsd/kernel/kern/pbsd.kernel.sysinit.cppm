module;
#include <cstdint>

export module pbsd.kernel.sysinit;

import pbsd.core;

/// Freestanding port of `sys/kernel.h` SYSINIT ordering tables.
export namespace pbsd::kernel::sysinit {

enum class SubId : unsigned {
    Dummy           = 0x0000000u,
    Tunables        = 0x0700000u,
    Copyright       = 0x0800001u,
    Vm              = 0x1000000u,
    Counter         = 0x1100000u,
    Kmem            = 0x1800000u,
    Witness         = 0x1A80000u,
    Lock            = 0x1B00000u,
    Eventhandler    = 0x1C00000u,
    Kld             = 0x2000000u,
    Intr            = 0x2800000u,
    Taskq           = 0x2880000u,
    Epoch           = 0x2888000u,
    Drivers         = 0x3100000u,
    Configure       = 0x3800000u,
    Vfs             = 0x4000000u,
    Syscalls        = 0xD800000u,
    Last            = 0xFFFFFFFu,
};

enum class Order : unsigned {
    First   = 0x0000000u,
    Second  = 0x0000001u,
    Third   = 0x0000002u,
    Fourth  = 0x0000003u,
    Fifth   = 0x0000004u,
    Middle  = 0x1000000u,
    Any     = 0xFFFFFFFu,
};

using InitFunc = void (*)(const void*);

struct Entry {
    SubId       subsystem{};
    Order       order{};
    InitFunc    func{nullptr};
    const void* udata{nullptr};
    const char* name{nullptr};
};

[[nodiscard]] constexpr unsigned sort_key(SubId sub, Order ord) noexcept {
    return (static_cast<unsigned>(sub) << 4)
         | (static_cast<unsigned>(ord) & 0xFu);
}

[[nodiscard]] constexpr bool precedes(const Entry& a, const Entry& b) noexcept {
    const unsigned ka = sort_key(a.subsystem, a.order);
    const unsigned kb = sort_key(b.subsystem, b.order);
    return ka < kb;
}

[[nodiscard]] inline Status validate_entry(const Entry& e) noexcept {
    if (e.func == nullptr || e.name == nullptr) {
        return Status::Invalid;
    }
    if (static_cast<unsigned>(e.subsystem) > static_cast<unsigned>(SubId::Last)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status run_sorted(Entry* table, unsigned count) noexcept {
    if (table == nullptr) {
        return Status::Invalid;
    }
    for (unsigned i = 0; i < count; ++i) {
        if (validate_entry(table[i]) != Status::Ok) {
            return Status::Invalid;
        }
    }
    for (unsigned i = 0; i + 1 < count; ++i) {
        if (!precedes(table[i], table[i + 1])) {
            return Status::Denied;
        }
    }
    for (unsigned i = 0; i < count; ++i) {
        table[i].func(table[i].udata);
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned count_runnable(const Entry* table, unsigned count) noexcept {
    if (table == nullptr) {
        return 0;
    }
    unsigned n = 0;
    for (unsigned i = 0; i < count; ++i) {
        if (table[i].func != nullptr && table[i].name != nullptr) {
            ++n;
        }
    }
    return n;
}

struct SubsystemName {
    SubId       id;
    const char* name;
};

inline constexpr SubsystemName kSubsystemNames[] = {
    {SubId::Tunables,     "tunables"},
    {SubId::Witness,      "witness"},
    {SubId::Lock,         "lock"},
    {SubId::Eventhandler, "eventhandler"},
    {SubId::Taskq,        "taskqueue"},
    {SubId::Intr,         "intr"},
    {SubId::Epoch,        "epoch"},
    {SubId::Drivers,      "drivers"},
    {SubId::Configure,    "configure"},
    {SubId::Vfs,          "vfs"},
    {SubId::Syscalls,     "syscalls"},
};

[[nodiscard]] inline const char* subsystem_name(SubId id) noexcept {
    for (const auto& e : kSubsystemNames) {
        if (e.id == id) {
            return e.name;
        }
    }
    return nullptr;
}

inline constexpr Entry kBootstrapTable[] = {
    {SubId::Tunables,     Order::First,  nullptr, nullptr, "tunables"},
    {SubId::Witness,      Order::Middle, nullptr, nullptr, "witness"},
    {SubId::Lock,         Order::Middle, nullptr, nullptr, "lock"},
    {SubId::Eventhandler, Order::Middle, nullptr, nullptr, "eventhandler"},
    {SubId::Taskq,        Order::Second, nullptr, nullptr, "taskqueue"},
    {SubId::Intr,         Order::First,  nullptr, nullptr, "intr"},
    {SubId::Epoch,        Order::First,  nullptr, nullptr, "epoch"},
    {SubId::Drivers,      Order::Any,    nullptr, nullptr, "drivers"},
    {SubId::Configure,    Order::Any,    nullptr, nullptr, "configure"},
    {SubId::Vfs,          Order::Any,    nullptr, nullptr, "vfs"},
    {SubId::Syscalls,     Order::Any,    nullptr, nullptr, "syscalls"},
};

[[nodiscard]] inline unsigned bootstrap_table_size() noexcept {
    return static_cast<unsigned>(sizeof(kBootstrapTable) / sizeof(kBootstrapTable[0]));
}

} // namespace pbsd::kernel::sysinit
