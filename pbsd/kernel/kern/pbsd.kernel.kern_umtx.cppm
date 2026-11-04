module;

export module pbsd.kernel.kern_umtx;

export import pbsd.core;

/// Freestanding port of `kern/kern_umtx.c` — umtx helpers.
export namespace pbsd::kernel::kern_umtx {

inline constexpr int kOpWait = 2;
inline constexpr int kOpWake = 3;
inline constexpr int kOpMutexLock = 5;
inline constexpr int kOpMutexUnlock = 6;
inline constexpr int kAbstime = 0x01;

[[nodiscard]] inline Status validate_op(int op) noexcept {
    switch (op) {
    case kOpWait:
    case kOpWake:
    case kOpMutexLock:
    case kOpMutexUnlock:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::kernel::kern_umtx
