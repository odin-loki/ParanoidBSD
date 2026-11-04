module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.lockf;

export import pbsd.core;

/// lockf from hbsd/src/lib/libc/gen/lockf.c
export namespace pbsd::userland::libc {

enum class LockOp : int { Unlock = 0, Lock, Test, TestLock };

[[nodiscard]] inline Status lockf_op(LockOp op) noexcept {
    (void)op;
    return Status::Ok;
}

} // namespace pbsd::userland::libc
