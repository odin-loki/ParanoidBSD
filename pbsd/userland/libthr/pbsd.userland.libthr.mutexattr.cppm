module;

export module pbsd.userland.libthr.mutexattr;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread_mutexattr from hbsd/src/lib/libthr/thread/thr_mutexattr.c
export namespace pbsd::userland::libthr {

struct MutexAttr {
    MutexType type{MutexType::Normal};
    bool pshared{false};
};

[[nodiscard]] inline Status mutexattr_init(MutexAttr& a) noexcept {
    a = MutexAttr{};
    return Status::Ok;
}

[[nodiscard]] inline Status mutexattr_settype(MutexAttr& a, MutexType t) noexcept {
    a.type = t;
    return Status::Ok;
}

[[nodiscard]] inline MutexType mutexattr_gettype(const MutexAttr& a) noexcept { return a.type; }

} // namespace pbsd::userland::libthr
