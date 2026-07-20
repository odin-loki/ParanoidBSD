module;

export module pbsd.userland.libthr.single_np;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread_single_np from hbsd/src/lib/libthr/thread/thr_single_np.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status single_np_enable(bool& enabled) noexcept {
    enabled = true;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
