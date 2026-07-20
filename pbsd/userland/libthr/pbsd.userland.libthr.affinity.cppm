module;

export module pbsd.userland.libthr.affinity;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// cpu affinity concepts from hbsd/src/lib/libthr/thread/thr_affinity.c
export namespace pbsd::userland::libthr {

struct CpuSet {
    unsigned mask{0};
};

[[nodiscard]] inline Status affinity_get(ThreadId tid, CpuSet& out) noexcept {
    if (tid == kInvalidThread) {
        return Status::Invalid;
    }
    out.mask = 1u;
    return Status::Ok;
}

[[nodiscard]] inline Status affinity_set(ThreadId tid, const CpuSet& set) noexcept {
    if (tid == kInvalidThread) {
        return Status::Invalid;
    }
    (void)set;
    return Status::NotImplemented;
}

} // namespace pbsd::userland::libthr
