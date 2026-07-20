module;
#include <concepts>

export module pbsd.kernel.sched;

export import pbsd.handles;
import pbsd.core;

/// Wave 4 — scheduler hooks (typed façade over hbsd sched KPI).
export namespace pbsd::kernel::sched {

/// SI-5 hosted scaffold — ISR context must not invoke blocking scheduler hooks.
inline thread_local bool in_isr_context{false};

struct IsrScopeGuard {
    IsrScopeGuard() noexcept { in_isr_context = true; }
    ~IsrScopeGuard() noexcept { in_isr_context = false; }
    IsrScopeGuard(const IsrScopeGuard&) = delete;
    IsrScopeGuard& operator=(const IsrScopeGuard&) = delete;
};

[[nodiscard]] inline Status require_thread_context() noexcept {
    return in_isr_context ? Status::Denied : Status::Ok;
}

enum class SchedClass : unsigned char {
    Idle      = 0,
    Timeshare = 1,
    Realtime  = 2,
    IdlePage  = 3,
};

struct ThreadObject {
    static void release(ThreadObject* p) noexcept { (void)p; }
};

using ThreadHandle = UniqueHandle<ThreadObject>;

struct SchedParams {
    SchedClass    cls{SchedClass::Timeshare};
    int           priority{0};
    unsigned long quantum_us{10000};
};

template<typename H>
concept SchedHook = requires(H h, ThreadHandle& t, SchedParams p) {
    { h.on_enqueue(t, p) } -> std::same_as<Status>;
    { h.on_dequeue(t) } -> std::same_as<Status>;
    { h.on_yield(t) } -> std::same_as<Status>;
};

class DefaultSchedHook {
public:
    [[nodiscard]] Status on_enqueue(ThreadHandle& t, SchedParams p) noexcept {
        if (require_thread_context() != Status::Ok) {
            return Status::Denied;
        }
        if (!t.valid()) {
            return Status::Invalid;
        }
        last_params_ = p;
        enqueued_ = true;
        return Status::Ok;
    }

    [[nodiscard]] Status on_dequeue(ThreadHandle& t) noexcept {
        if (require_thread_context() != Status::Ok) {
            return Status::Denied;
        }
        if (!t.valid() || !enqueued_) {
            return Status::Invalid;
        }
        enqueued_ = false;
        return Status::Ok;
    }

    [[nodiscard]] Status on_yield(ThreadHandle& t) noexcept {
        if (require_thread_context() != Status::Ok) {
            return Status::Denied;
        }
        if (!t.valid()) {
            return Status::Invalid;
        }
        return Status::Ok;
    }

    [[nodiscard]] SchedParams last_params() const noexcept { return last_params_; }

private:
    SchedParams last_params_{};
    bool        enqueued_{false};
};

static_assert(SchedHook<DefaultSchedHook>);

} // namespace pbsd::kernel::sched
