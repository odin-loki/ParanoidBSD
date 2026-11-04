module;
#include <cstdint>

export module pbsd.kernel.context;

import pbsd.core;

/// Freestanding port of `kern/kern_context.c` ucontext copy size helpers.
export namespace pbsd::kernel::context {

inline constexpr unsigned kUcCopySize = 128; // offsetof(ucontext_t, uc_link)

struct UcontextHeader {
    unsigned char sigmask[16]{};
    unsigned char mcontext[96]{};
};

[[nodiscard]] inline Status validate_copy_size(unsigned size) noexcept {
    if (size < kUcCopySize) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status copy_header(const UcontextHeader& src,
                                        UcontextHeader& dst) noexcept {
    for (unsigned i = 0; i < sizeof(src.sigmask); ++i) {
        dst.sigmask[i] = src.sigmask[i];
    }
    for (unsigned i = 0; i < sizeof(src.mcontext); ++i) {
        dst.mcontext[i] = src.mcontext[i];
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::context
