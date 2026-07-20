module;

export module pbsd.userland.libthr.barrierattr;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread_barrierattr from hbsd/src/lib/libthr/thread/thr_barrierattr.c
export namespace pbsd::userland::libthr {

struct BarrierAttr { bool pshared{false}; };

inline void barrierattr_init(BarrierAttr& a) noexcept { a = BarrierAttr{}; }

} // namespace pbsd::userland::libthr
