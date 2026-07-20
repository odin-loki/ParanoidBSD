module;

export module pbsd.userland.libthr.spec;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// thr_spec from hbsd/src/lib/libthr/thread/thr_spec.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status spec_set(int key, void* value) noexcept {
    (void)key;
    (void)value;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
