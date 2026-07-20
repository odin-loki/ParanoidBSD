module;

export module pbsd.kernel.subr_stack;

import pbsd.core;

/// Freestanding port of `kern/subr_stack.c` — stack trace capture helpers.
export namespace pbsd::kernel::subr_stack {

inline constexpr int kMaxDepth = 18;

[[nodiscard]] inline Status validate_depth(int depth) noexcept {
    if (depth < 0 || depth > kMaxDepth) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool is_full(int depth) noexcept {
    return depth >= kMaxDepth;
}

} // namespace pbsd::kernel::subr_stack
