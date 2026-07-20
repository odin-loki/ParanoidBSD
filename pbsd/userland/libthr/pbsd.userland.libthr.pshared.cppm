module;

export module pbsd.userland.libthr.pshared;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pshared attrs from hbsd/src/lib/libthr/thread/thr_pshared.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status pshared_set(bool& flag, bool value) noexcept {
    flag = value;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
