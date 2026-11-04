module;
#include <cstdint>

export module pbsd.kernel.systrace_args;

import pbsd.core;

/// Freestanding port of `kern/systrace_args.c` — syscall trace argument helpers.
export namespace pbsd::kernel::systrace_args {

inline constexpr unsigned kMaxArgs = 8;

struct TraceArg {
    unsigned index{};
    std::uint64_t value{};
};

[[nodiscard]] inline Status validate_index(unsigned idx) noexcept {
    return idx < kMaxArgs ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline Status set_arg(TraceArg& arg, unsigned idx, std::uint64_t value) noexcept {
    if (validate_index(idx) != Status::Ok) {
        return Status::Invalid;
    }
    arg.index = idx;
    arg.value = value;
    return Status::Ok;
}

} // namespace pbsd::kernel::systrace_args
