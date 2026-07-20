module;

export module pbsd.userland.rtld.lock;

import pbsd.core;

/// rtld lock from hbsd/src/libexec/rtld-elf/rtld_lock.c
export namespace pbsd::userland::rtld {

inline bool g_rtld_locked = false;

[[nodiscard]] inline Status rtld_lock() noexcept {
    if (g_rtld_locked) {
        return Status::Busy;
    }
    g_rtld_locked = true;
    return Status::Ok;
}

[[nodiscard]] inline Status rtld_unlock() noexcept {
    g_rtld_locked = false;
    return Status::Ok;
}

} // namespace pbsd::userland::rtld
