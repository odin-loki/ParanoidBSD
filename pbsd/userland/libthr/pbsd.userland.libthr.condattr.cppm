module;

export module pbsd.userland.libthr.condattr;

import pbsd.core;

/// pthread_condattr from hbsd/src/lib/libthr/thread/thr_condattr.c
export namespace pbsd::userland::libthr {

struct CondAttr {
    bool pshared{false};
    int clock_id{0};
};

[[nodiscard]] inline Status condattr_init(CondAttr& a) noexcept {
    a = CondAttr{};
    return Status::Ok;
}

[[nodiscard]] inline Status condattr_setpshared(CondAttr& a, bool pshared) noexcept {
    a.pshared = pshared;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
