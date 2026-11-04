module;

#if defined(_SC_TIMEOUTS)
#include <unistd.h>
#endif

export module pbsd.kde.frameworks.kcoreaddons.ksdclock;

import pbsd.core;

/// Wave 3 — shared cache lock selection (from findBestSharedLock in ksdclock.cpp).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/caching/ksdclock.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::ksdclock {

enum class SharedLockId : unsigned {
    Mutex = 0,
    Semaphore,
    Spinlock,
};

[[nodiscard]] inline bool timeouts_supported() noexcept {
#if defined(_SC_TIMEOUTS)
    return ::sysconf(_SC_TIMEOUTS) >= 200112L;
#else
    return false;
#endif
}

[[nodiscard]] inline SharedLockId find_best_shared_lock() noexcept {
    const bool timeouts = timeouts_supported();
#if defined(__FreeBSD__)
    if (timeouts) {
        return SharedLockId::Mutex;
    }
    return SharedLockId::Mutex;
#else
    (void)timeouts;
    return SharedLockId::Spinlock;
#endif
}

[[nodiscard]] inline const char* lock_id_name(SharedLockId id) noexcept {
    switch (id) {
    case SharedLockId::Mutex:
        return "pthread_mutex";
    case SharedLockId::Semaphore:
        return "semaphore";
    case SharedLockId::Spinlock:
        return "spinlock";
    }
    return "unknown";
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/caching/ksdclock.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::ksdclock
