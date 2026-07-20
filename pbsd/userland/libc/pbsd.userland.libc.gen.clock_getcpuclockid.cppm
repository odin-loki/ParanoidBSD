module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.clock_getcpuclockid;

export import pbsd.core;

/// clock_getcpuclockid from hbsd/src/lib/libc/gen/clock_getcpuclockid.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status clock_getcpuclockid_pid(int pid) noexcept {
    if (pid < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
