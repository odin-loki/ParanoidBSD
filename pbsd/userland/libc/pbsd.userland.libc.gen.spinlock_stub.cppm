module;

export module pbsd.userland.libc.gen.spinlock_stub;

export import pbsd.core;
import pbsd.userland.libc.gen.spinlock_stub_impl;

/// spinlock_stub from hbsd/src/lib/libc/gen/_spinlock_stub.c
export namespace pbsd::userland::libc::gen {

[[nodiscard]] inline bool spinlock_stub_active() noexcept {
    return pbsd::userland::libc::spinlock_stub_impl_ok() == Status::Ok;
}

} // namespace pbsd::userland::libc::gen
