module;

export module pbsd.userland.libthr.rwlockattr;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread_rwlockattr from hbsd/src/lib/libthr/thread/thr_rwlockattr.c
export namespace pbsd::userland::libthr {

struct RwLockAttr { bool pshared{false}; };

inline void rwlockattr_init(RwLockAttr& a) noexcept { a = RwLockAttr{}; }

} // namespace pbsd::userland::libthr
