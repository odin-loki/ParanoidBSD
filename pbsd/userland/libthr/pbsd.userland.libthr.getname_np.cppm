module;
#include <cstddef>

export module pbsd.userland.libthr.getname_np;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread_getname_np from hbsd/src/lib/libthr/thread/thr_setname.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status getname_np(ThreadId tid, char* name, std::size_t len) noexcept {
    if (tid == kInvalidThread || name == nullptr || len == 0) {
        return Status::Invalid;
    }
    name[0] = '\0';
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
