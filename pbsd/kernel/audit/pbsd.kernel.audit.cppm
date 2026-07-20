module;
#include <cstdint>

export module pbsd.kernel.audit;

export import pbsd.core;

/// Wave 4 — audit subsystem kernel hooks (sys/security/audit/audit.h, bsm/audit.h).
export namespace pbsd::kernel::audit {

inline constexpr unsigned kRecordMagic       = 0x828a0f1b;
inline constexpr unsigned kMaxAuditRecords   = 20;
inline constexpr unsigned kMaxAuditRecordSize = 0x8000 - 1;

inline constexpr int kTriggerLowSpace        = 1;
inline constexpr int kTriggerRotateKernel    = 2;
inline constexpr int kTriggerReadFile        = 3;
inline constexpr int kTriggerCloseAndDie     = 4;
inline constexpr int kTriggerNoSpace         = 5;
inline constexpr int kTriggerRotateUser      = 6;
inline constexpr int kTriggerInitialize      = 7;
inline constexpr int kTriggerExpireTrails    = 8;

inline constexpr int kAueNull   = 0;
inline constexpr int kAueExit   = 1;
inline constexpr int kAueFork   = 2;
inline constexpr int kAueOpen   = 3;
inline constexpr int kAueExecve = 23;

struct AuditState {
    bool trail_enabled{};
    bool trail_suspended{};
    bool syscalls_enabled{};
    bool dtrace_enabled{};
};

struct EventEntry {
    int         aue_id{};
    const char* name{};
    bool        syscall_class{};
};

inline constexpr EventEntry kEventTable[] = {
    {kAueNull,   "AUE_NULL",   false},
    {kAueExit,   "AUE_EXIT",   true},
    {kAueFork,   "AUE_FORK",   true},
    {kAueOpen,   "AUE_OPEN",   true},
    {kAueExecve, "AUE_EXECVE", true},
};

[[nodiscard]] inline unsigned event_table_size() noexcept {
    return static_cast<unsigned>(sizeof(kEventTable) / sizeof(kEventTable[0]));
}

[[nodiscard]] constexpr bool is_kevent(int e) noexcept {
    return (e > 0 && e < 2048);
}

[[nodiscard]] constexpr Status validate_trigger(int trig) noexcept {
    if (trig >= kTriggerLowSpace && trig <= kTriggerExpireTrails) {
        return Status::Ok;
    }
    return Status::Invalid;
}

[[nodiscard]] constexpr bool should_audit_syscall(const AuditState& st,
                                                  int aue) noexcept {
    if (!st.trail_enabled || st.trail_suspended) {
        return false;
    }
    if (!st.syscalls_enabled) {
        return false;
    }
    return is_kevent(aue);
}

[[nodiscard]] constexpr Status validate_record_size(unsigned len) noexcept {
    if (len == 0 || len > kMaxAuditRecordSize) {
        return Status::Invalid;
    }
    return Status::Ok;
}

enum class Token : unsigned {
    Invalid = 0,
    Data = 1,
    Text = 2,
    Path = 3,
    Subject = 14,
    Process = 15,
    Arg = 16,
    Return32 = 17,
    Priv = 26,
    Right = 35,
};

struct PipeStub {
    unsigned qlen{};
    unsigned qlimit{64};
    bool async{};
};

[[nodiscard]] constexpr Status validate_pipe(const PipeStub& p) noexcept {
    if (p.qlen > p.qlimit) {
        return Status::Protocol;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned token_table_size() noexcept {
    return 10;
}

} // namespace pbsd::kernel::audit
