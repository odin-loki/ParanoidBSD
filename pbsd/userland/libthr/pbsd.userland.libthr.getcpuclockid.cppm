module;
#include <cstdint>

export module pbsd.userland.libthr.getcpuclockid;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread_getcpuclockid from hbsd/src/lib/libthr/thread/thr_getcpuclockid.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Result<std::uint64_t> getcpuclockid_thread(ThreadId tid) noexcept {
    if (tid == 0) {
        return result_err<std::uint64_t>(Status::Invalid);
    }
    return result_ok<std::uint64_t>(0);
}

} // namespace pbsd::userland::libthr
