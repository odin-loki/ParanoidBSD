module;

export module pbsd.userland.libthr.printf;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// thr_printf from hbsd/src/lib/libthr/thread/thr_printf.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status printf_lock() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libthr
