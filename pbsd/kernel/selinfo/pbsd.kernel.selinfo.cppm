module;
#include <cstddef>
#include <cstdint>

export module pbsd.kernel.selinfo;

export import pbsd.core;
export import pbsd.kernel.kevent;

/// Wave 4 — select/poll wait queues (sys/selinfo.h) freestanding stubs.
export namespace pbsd::kernel::selinfo {

struct SelfdStub {
    void* next{};
    void* thread{};
};

struct SelinfoStub {
    void* si_tdlist_head{};
    void* si_note{};
    void* si_mtx{};
    std::size_t waiter_count{};
};

[[nodiscard]] constexpr bool sel_waiting(const SelinfoStub& si) noexcept {
    return si.waiter_count > 0 || si.si_tdlist_head != nullptr;
}

[[nodiscard]] constexpr Status sel_record(SelinfoStub& si) noexcept {
    ++si.waiter_count;
    return Status::Ok;
}

[[nodiscard]] constexpr Status sel_wakeup(SelinfoStub& si) noexcept {
    if (!sel_waiting(si)) {
        return Status::NotFound;
    }
    si.waiter_count = 0;
    si.si_tdlist_head = nullptr;
    return Status::Ok;
}

[[nodiscard]] constexpr Status sel_drain(SelinfoStub& si) noexcept {
    si.waiter_count = 0;
    si.si_tdlist_head = nullptr;
    si.si_note = nullptr;
    return Status::Ok;
}

[[nodiscard]] constexpr bool poll_events_from_kevent(int filter,
                                                     unsigned fflags) noexcept {
    using namespace pbsd::kernel::kevent;
    if (filter == kEvFiltRead) {
        return true;
    }
    if (filter == kEvFiltWrite) {
        return true;
    }
    if (filter == kEvFiltVnode && (fflags & kNoteDelete) != 0) {
        return true;
    }
    return false;
}

[[nodiscard]] constexpr Status validate_selinfo(const SelinfoStub& si) noexcept {
    if (si.waiter_count == 0 && si.si_tdlist_head != nullptr) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::selinfo
