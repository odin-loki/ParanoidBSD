module;

export module pbsd.userland.libthr.fork;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread_atfork from hbsd/src/lib/libthr/thread/thr_fork.c
export namespace pbsd::userland::libthr {

using AtForkHandler = void (*)() noexcept;

struct AtForkHandlers {
    AtForkHandler prepare{nullptr};
    AtForkHandler parent{nullptr};
    AtForkHandler child{nullptr};
};

[[nodiscard]] inline Status atfork_register(AtForkHandlers& h, AtForkHandler prepare,
                                            AtForkHandler parent,
                                            AtForkHandler child) noexcept {
    h.prepare = prepare;
    h.parent = parent;
    h.child = child;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
