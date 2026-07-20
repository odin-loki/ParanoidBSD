module;

#include <cstddef>

export module pbsd.userland.libthr.attr;

import pbsd.core;
import pbsd.userland.libthr.thread;

/// pthread_attr_* from hbsd/src/lib/libthr/thread/thr_attr.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status attr_init(ThreadAttr& attr) noexcept {
    attr = ThreadAttr{};
    return Status::Ok;
}

[[nodiscard]] inline Status attr_destroy(ThreadAttr&) noexcept { return Status::Ok; }

[[nodiscard]] inline Status attr_setstacksize(ThreadAttr& attr, unsigned size) noexcept {
    attr.stack_size = size;
    return Status::Ok;
}

[[nodiscard]] inline Status attr_getstacksize(const ThreadAttr& attr, unsigned& size) noexcept {
    size = attr.stack_size;
    return Status::Ok;
}

[[nodiscard]] inline Status attr_setdetachstate(ThreadAttr& attr, bool detached) noexcept {
    attr.detached = detached;
    return Status::Ok;
}

[[nodiscard]] inline Status attr_getdetachstate(const ThreadAttr& attr, bool& detached) noexcept {
    detached = attr.detached;
    return Status::Ok;
}

[[nodiscard]] inline Status attr_setguardsize(ThreadAttr& attr, std::size_t size) noexcept {
    (void)attr;
    (void)size;
    return Status::Ok;
}

[[nodiscard]] inline Status attr_getguardsize(const ThreadAttr& attr, std::size_t& size) noexcept {
    (void)attr;
    size = 0;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
