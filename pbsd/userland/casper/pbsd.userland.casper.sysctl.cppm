export module pbsd.userland.casper.sysctl;

export import pbsd.userland.casper;
import pbsd.core;

/// cap_sysctl service surface from hbsd/src/lib/libcasper/services/cap_sysctl/
export namespace pbsd::userland::casper::sysctl {

inline constexpr const char* kServiceName = "system.sysctl";

inline constexpr int CAP_SYSCTL_READ      = 0x01;
inline constexpr int CAP_SYSCTL_WRITE     = 0x02;
inline constexpr int CAP_SYSCTL_RDWR      = (CAP_SYSCTL_READ | CAP_SYSCTL_WRITE);
inline constexpr int CAP_SYSCTL_RECURSIVE = 0x04;

struct SysctlLimitObject {
    ChannelHandle* chan{nullptr};
    unsigned entry_count{0};

    static void release(SysctlLimitObject* p) noexcept {
        if (p != nullptr) {
            p->chan = nullptr;
            p->entry_count = 0;
        }
    }
};

using SysctlLimitHandle = UniqueHandle<SysctlLimitObject>;

[[nodiscard]] inline bool flags_valid(int flags) noexcept {
    const int mask = CAP_SYSCTL_READ | CAP_SYSCTL_WRITE | CAP_SYSCTL_RECURSIVE;
    return (flags & ~mask) == 0 && (flags & CAP_SYSCTL_RDWR) != 0;
}

[[nodiscard]] inline Result<SysctlLimitHandle>
limit_init(ChannelHandle& chan, LineageTree& tree,
           CapabilityRights rights) noexcept {
    if (!chan.valid() || !channel_valid(chan.peek())) {
        return {Status::Invalid, SysctlLimitHandle{}};
    }
    const LineageId id = tree.create_root();
    if (id == kInvalidLineage) {
        return {Status::NoMemory, SysctlLimitHandle{}};
    }
    auto* obj = new SysctlLimitObject{};
    obj->chan = &chan;
    return {Status::Ok, SysctlLimitHandle{obj, rights, id}};
}

[[nodiscard]] inline Status limit_name(SysctlLimitHandle& limit,
                                       const char* name, int flags) noexcept {
    if (!limit.valid()) {
        return Status::Invalid;
    }
    if (name == nullptr || name[0] == '\0') {
        return Status::Invalid;
    }
    if (!flags_valid(flags)) {
        return Status::Invalid;
    }
    ++limit.peek()->entry_count;
    return Status::Ok;
}

[[nodiscard]] inline Status limit_mib(SysctlLimitHandle& limit,
                                      const int* mib, unsigned miblen,
                                      int flags) noexcept {
    if (!limit.valid()) {
        return Status::Invalid;
    }
    if (mib == nullptr || miblen == 0) {
        return Status::Invalid;
    }
    if (!flags_valid(flags)) {
        return Status::Invalid;
    }
    ++limit.peek()->entry_count;
    return Status::Ok;
}

[[nodiscard]] inline Result<ChannelHandle>
open_sysctl_service(LineageTree& tree, CapabilityRights rights) noexcept {
    return open_service(tree, rights, kServiceName);
}

[[nodiscard]] inline Status limit_apply(SysctlLimitHandle& limit) noexcept {
    if (!limit.valid()) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::casper::sysctl
