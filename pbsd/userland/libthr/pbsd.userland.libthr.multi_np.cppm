module;

export module pbsd.userland.libthr.multi_np;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread_multi_np from hbsd/src/lib/libthr/thread/thr_multi_np.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status multi_np_enable(bool& enabled) noexcept {
    enabled = true;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
