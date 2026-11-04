module;
#include <cstdint>

export module pbsd.kernel.stack;

export import pbsd.core;

/// Freestanding port of `sys/stack.h` / `kern/subr_stack.c`.
export namespace pbsd::kernel::stack {

inline constexpr int kMaxDepth = 18;

enum class SbufFmt : int {
    None = 0,
    Long = 1,
    Compact = 2,
};

struct Softc {
    int depth{0};
    std::uint64_t pcs[kMaxDepth]{};
};

[[nodiscard]] inline Status put(Softc& st, std::uint64_t pc) noexcept {
    if (st.depth >= kMaxDepth) {
        return Status::NoMemory;
    }
    st.pcs[st.depth++] = pc;
    return Status::Ok;
}

inline void zero(Softc& st) noexcept {
    st.depth = 0;
}

} // namespace pbsd::kernel::stack
