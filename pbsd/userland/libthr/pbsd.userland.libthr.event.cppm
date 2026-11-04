module;

export module pbsd.userland.libthr.event;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// thr_event from hbsd/src/lib/libthr/thread/thr_event.c
export namespace pbsd::userland::libthr {

struct ThreadEvent { bool pending{false}; };

inline void event_signal(ThreadEvent& e) noexcept { e.pending = true; }

} // namespace pbsd::userland::libthr
