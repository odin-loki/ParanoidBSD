module;

export module pbsd.userland.libthr.once;

import pbsd.core;

/// pthread_once from hbsd/src/lib/libthr/thread/thr_once.c
export namespace pbsd::userland::libthr {

struct OnceFlag {
    bool done{false};
};

[[nodiscard]] inline Status once(OnceFlag& flag, void (*init)(void)) noexcept {
    if (!flag.done && init != nullptr) {
        init();
        flag.done = true;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
