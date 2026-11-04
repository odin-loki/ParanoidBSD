export module pbsd.kernel.process;

export import pbsd.handles;
import pbsd.core;
import pbsd.kernel.capsicum;
import pbsd.kernel.capsicum_bridge;
import pbsd.kernel.prot;

/// Process / FD / capability façade over hbsd proc + file KPI.
export namespace pbsd::kernel::process {

struct ProcessObject {
    static void release(ProcessObject* p) noexcept { (void)p; }
};

struct FdObject {
    static void release(FdObject* p) noexcept { (void)p; }
};

using ProcessHandle = UniqueHandle<ProcessObject>;
using FdHandle      = UniqueHandle<FdObject>;

enum class ProcCapability : unsigned char {
    Fork   = 1,
    Exec   = 2,
    Signal = 3,
};

[[nodiscard]] inline Status check_proc_cap(const ProcessHandle& proc,
                                           ProcCapability cap) noexcept {
    if (!proc.valid()) {
        return Status::Invalid;
    }
    switch (cap) {
    case ProcCapability::Fork:
        return proc.has_right(CapabilityRights::Grant) ? Status::Ok : Status::Denied;
    case ProcCapability::Exec:
        return proc.has_right(CapabilityRights::Execute) ? Status::Ok : Status::Denied;
    case ProcCapability::Signal:
        return proc.has_right(CapabilityRights::Write) ? Status::Ok : Status::Denied;
    }
    return Status::Invalid;
}

[[nodiscard]] inline Result<FdHandle> dup_fd(const FdHandle& fd,
                                             CapabilityRights want) noexcept {
    if (!fd.valid()) {
        return {Status::Invalid, FdHandle{}};
    }
    CapabilityRights narrowed = narrow_rights(fd.rights(), want);
    if (narrowed == CapabilityRights::None) {
        return {Status::Denied, FdHandle{}};
    }
    return {Status::Ok, FdHandle{fd.peek(), narrowed, fd.lineage()}};
}

/// `kern_cap_rights_limit` bridge — cap word + typed rights narrowing.
[[nodiscard]] inline Status limit_fd(const FdHandle& fd, CapabilityRights want,
                                     capsicum::CapRights& cap_out) noexcept {
    return pbsd::kernel::limit_handle_rights(fd.rights(), want, cap_out);
}

/// `sys_cap_enter` bridge.
[[nodiscard]] inline Status enter_capability_mode(const ProcessHandle& proc,
                                                  ::pbsd::kernel::ProcCapNamespace& ns,
                                                  LineageId root,
                                                  const capsicum::CapRights& rights) noexcept {
    if (check_proc_cap(proc, ProcCapability::Exec) != Status::Ok) {
        return Status::Denied;
    }
    return pbsd::kernel::enter_capsicum(ns, root, rights);
}

/// `p_cansee` typed wrapper — uses prot module policy defaults.
[[nodiscard]] inline Status can_see_process(const prot::Ucred& viewer,
                                            const prot::Ucred& target,
                                            const prot::BsdVisibilityPolicy& pol) noexcept {
    return prot::p_cansee(viewer, target, pol);
}

} // namespace pbsd::kernel::process
