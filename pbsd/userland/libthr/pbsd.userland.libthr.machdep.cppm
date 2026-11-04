module;
#include <cstdint>

export module pbsd.userland.libthr.machdep;

import pbsd.core;
export import pbsd.userland.libthr.mutex;
import pbsd.userland.libthr.thread;

/// Port of hbsd/src/lib/libthr/thread/thr/machdep.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline ThreadId machdep_self() noexcept { return thread_self(); }

} // namespace pbsd::userland::libthr
