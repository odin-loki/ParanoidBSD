module;

export module pbsd.userland.libthr.self;

import pbsd.core;
import pbsd.userland.libthr.mutex;
import pbsd.userland.libthr.thread;

/// pthread_self from hbsd/src/lib/libthr/thread/thr_self.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline ThreadId pthread_self_id() noexcept { return thread_self(); }

} // namespace pbsd::userland::libthr
