module;
#include <cstdint>

export module pbsd.kernel.rctl;

export import pbsd.core;

/// Wave 4/5 — resource limits control from sys/rctl.h, kern/kern_rctl.c.
export namespace pbsd::kernel::rctl {

inline constexpr unsigned kRctlRulePersist = 0x0001;
inline constexpr unsigned kRctlRuleInherit = 0x0002;
inline constexpr unsigned kRctlRuleLog     = 0x0004;

inline constexpr int kRctlActionAllow = 0;
inline constexpr int kRctlActionDeny  = 1;
inline constexpr int kRctlActionLog   = 2;

inline constexpr int kRctlSubjectUser   = 0;
inline constexpr int kRctlSubjectLogin  = 1;
inline constexpr int kRctlSubjectJail   = 2;
inline constexpr int kRctlSubjectProcess = 3;

struct RctlRule {
    int         subject_type{kRctlSubjectProcess};
    const char* subject{};
    const char* resource{};
    int         action{kRctlActionAllow};
    std::uint64_t amount{};
    unsigned    flags{};
};

struct RctlFlagEntry {
    unsigned    flag;
    const char* name;
};

inline constexpr RctlFlagEntry kFlagTable[] = {
    {kRctlRulePersist, "persist"},
    {kRctlRuleInherit, "inherit"},
    {kRctlRuleLog,     "log"},
};

[[nodiscard]] inline unsigned flag_table_size() noexcept {
    return static_cast<unsigned>(sizeof(kFlagTable) / sizeof(kFlagTable[0]));
}

[[nodiscard]] constexpr Status validate_action(int action) noexcept {
    if (action < kRctlActionAllow || action > kRctlActionLog) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr Status validate_subject(int subject) noexcept {
    if (subject < kRctlSubjectUser || subject > kRctlSubjectProcess) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::rctl
