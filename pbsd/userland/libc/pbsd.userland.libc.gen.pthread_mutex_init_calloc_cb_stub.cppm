module;

export module pbsd.userland.libc.gen.pthread_mutex_init_calloc_cb_stub;

export import pbsd.core;
import pbsd.userland.libc.gen.pthread_mutex_init_calloc_cb_stub_stub;

/// __pthread_mutex_init_calloc_cb_stub from hbsd/src/lib/libc/gen/__pthread_mutex_init_calloc_cb_stub.c
export namespace pbsd::userland::libc::gen {

[[nodiscard]] inline bool pthread_mutex_init_calloc_cb_stub_active() noexcept {
    return pbsd::userland::libc::pthread_mutex_init_calloc_cb_stub_stub_ok() == Status::Ok;
}

} // namespace pbsd::userland::libc::gen
